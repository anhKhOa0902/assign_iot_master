#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Arduino_MQTT_Client.h>
#include <ThingsBoard.h>
#include "../include/displayOLED.h"

// Định nghĩa chân LED và RELAY
#define LED_PIN 48
#define RELAY_PIN 6  // Relay cũ (nếu còn sử dụng)

// Định nghĩa chân cho module relay 2 kênh
#define RELAY_MODULE_CH1 1  // Kênh 1 cho máy bơm, kết nối với GPIO 7 (D4)
#define RELAY_MODULE_CH2 2  // Kênh 2 (dự phòng), có thể đặt GPIO khác nếu muốn sử dụng sau này

// Thông tin WiFi
const char* WIFI_SSID = "";
const char* WIFI_PASSWORD = "";

// Thông tin ThingsBoard
const char* THINGSBOARD_SERVER = "app.coreiot.io";
const char* TOKEN = "";
constexpr uint16_t THINGSBOARD_PORT = 1883U;

// Kích thước buffer MQTT
constexpr uint16_t MAX_MESSAGE_SIZE = 256U;

// Tốc độ baud của Serial
constexpr uint32_t SERIAL_BAUD = 115200U;

// Thời gian gửi telemetry
constexpr int16_t TELEMETRY_INTERVAL = 8000U;

// WiFi và ThingsBoard client
WiFiClient espClient;
Arduino_MQTT_Client mqttClient(espClient);
ThingsBoard tb(mqttClient, MAX_MESSAGE_SIZE, MAX_MESSAGE_SIZE);

// Cấu trúc dữ liệu nhận từ Slave
typedef struct struct_message {
  float temperature;
  float humidity;
  int soilMoisture;
} struct_message;

struct_message myData;

// Biến toàn cục để lưu dữ liệu
float temperature = 0.0;
float humidity = 0.0;
int soilMoisture = 0;
SemaphoreHandle_t dataMutex;

// Biến điều khiển LED và RELAY
volatile bool ledState = false;
volatile bool relayState = false;
volatile bool pumpState = false; // Trạng thái máy bơm (relay kênh 1)

// Điều khiển máy bơm - chỉ chế độ thủ công
RPC_Response setPumpState(const RPC_Data &data) {
  Serial.println("Received pump control from ThingsBoard");
  bool newState = data;
  
  Serial.print("Pump state change: ");
  Serial.println(newState ? "ON" : "OFF");
  digitalWrite(RELAY_MODULE_CH1, newState ? HIGH : LOW);
  pumpState = newState;
  
  return RPC_Response("setPumpState", pumpState);
}

// Callback khi nhận dữ liệu từ Slave
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  Serial.println("OnDataRecv triggered!");
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.println("Data received from Slave!");
  Serial.print("Temperature: ");
  Serial.print(myData.temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(myData.humidity);
  Serial.print(" %, Soil Moisture: ");
  Serial.print(myData.soilMoisture);
  Serial.println(" %");

  if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
    temperature = myData.temperature;
    humidity = myData.humidity;
    soilMoisture = myData.soilMoisture;
    xSemaphoreGive(dataMutex);
  } else {
    Serial.println("Failed to take mutex in OnDataRecv!");
  }

  // Cập nhật màn hình OLED
  displaySensorData(temperature, humidity, soilMoisture);
  
  // Không có điều khiển tự động trong phiên bản này
}

// RPC callback để điều khiển LED
RPC_Response setLedSwitchState(const RPC_Data &data) {
  Serial.println("Received LED switch state from ThingsBoard");
  bool newState = data;
  Serial.print("LED state change: ");
  Serial.println(newState);
  digitalWrite(LED_PIN, newState);
  ledState = newState;
  return RPC_Response("setLedSwitchValue", newState);
}

// RPC callback để điều khiển RELAY cũ (nếu còn sử dụng)
RPC_Response setRelaySwitchState(const RPC_Data &data) {
  Serial.println("Received Relay switch state from ThingsBoard");
  bool newState = data;
  Serial.print("Relay state change: ");
  Serial.println(newState);
  digitalWrite(RELAY_PIN, newState);
  relayState = newState;
  return RPC_Response("setRelaySwitchValue", newState);
}

// Thêm callback relay và pump vào mảng
const std::array<RPC_Callback, 3U> callbacks = {
  RPC_Callback{ "setLedSwitchValue", setLedSwitchState },
  RPC_Callback{ "setRelaySwitchValue", setRelaySwitchState },
  RPC_Callback{ "setPumpState", setPumpState }
};

// Task gửi dữ liệu lên ThingsBoard
void taskSendToThingsBoard(void *pvParameters) {
  (void)pvParameters;

  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(500 / portTICK_PERIOD_MS);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  
  // Lấy kênh WiFi hiện tại và in ra để debug
  int wifiChannel = WiFi.channel();
  Serial.print("WiFi Channel: ");
  Serial.println(wifiChannel);

  while (1) {
    if (!tb.connected()) {
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi disconnected, reconnecting...");
        WiFi.reconnect();
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        continue;
      }
      Serial.printf("Connecting to ThingsBoard (%s) with token (%s)\n", 
                    THINGSBOARD_SERVER, TOKEN);
      if (!tb.connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT)) {
        Serial.println("Failed to connect to ThingsBoard");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        continue;
      }
      Serial.println("Connected to ThingsBoard");

      if (!tb.RPC_Subscribe(callbacks.cbegin(), callbacks.cend())) {
        Serial.println("Failed to subscribe for RPC");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        continue;
      }
      Serial.println("Subscribed for RPC");
    }

    float tempToSend = 0.0;
    float humToSend = 0.0;
    int soilToSend = 0; 
    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
      tempToSend = temperature;
      humToSend = humidity;
      soilToSend = soilMoisture; 
      xSemaphoreGive(dataMutex);
    } else {
      Serial.println("Failed to take mutex in taskSendToThingsBoard!");
    }

    Serial.println("Sending data to ThingsBoard...");
    // Gửi dữ liệu cảm biến
    tb.sendTelemetryData("temperature", tempToSend);
    tb.sendTelemetryData("humidity", humToSend);
    tb.sendTelemetryData("soilMoisture", soilToSend);
    
    // Gửi trạng thái của LED, RELAY cũ và máy bơm
    tb.sendTelemetryData("ledState", ledState);
    tb.sendTelemetryData("relayState", relayState);
    tb.sendTelemetryData("pumpState", pumpState);

    tb.loop();
    vTaskDelay(TELEMETRY_INTERVAL / portTICK_PERIOD_MS);
  }
}

// Task xử lý RPC
void taskHandleRPC(void *pvParameters) {
  (void)pvParameters;
  while (1) {
    if (tb.connected()) {
      tb.loop();
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(1000);
  Serial.println("Master ESP32 Starting...");

  // Cấu hình GPIO
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(RELAY_MODULE_CH1, OUTPUT);
  pinMode(RELAY_MODULE_CH2, OUTPUT);
  
  // Đặt trạng thái ban đầu là OFF
  digitalWrite(LED_PIN, LOW);
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(RELAY_MODULE_CH1, LOW);
  digitalWrite(RELAY_MODULE_CH2, LOW);

  initOLED();

  // Kết nối WiFi trước
  Serial.println("Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  
  // Lấy kênh WiFi hiện tại - quan trọng cho ESP-NOW
  int wifiChannel = WiFi.channel();
  Serial.print("Master MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Operating on WiFi channel: ");
  Serial.println(wifiChannel);

  // Bây giờ khởi tạo ESP-NOW sau khi đã kết nối WiFi
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  Serial.println("ESP-NOW initialized");

  // Đăng ký callback khi nhận dữ liệu
  esp_now_register_recv_cb(OnDataRecv);

  dataMutex = xSemaphoreCreateMutex();

  xTaskCreate(taskSendToThingsBoard, "sendThingsBoard", 8192, NULL, 1, NULL);
  xTaskCreate(taskHandleRPC, "handleRPC", 4096, NULL, 2, NULL);

  // Xóa task chính (setup) vì đang sử dụng FreeRTOS
  vTaskDelete(NULL);
}

void loop() {
  // Không cần code trong loop vì đang sử dụng FreeRTOS
}