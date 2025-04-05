#include "../include/displayOLED.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void initOLED() {
  Wire.begin(11, 12); // SDA = GPIO11, SCL = GPIO12
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Địa chỉ I2C mặc định: 0x3C
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Treo nếu OLED không khởi tạo được
  }
  display.clearDisplay();
  display.display();
}

void displaySensorData(float temperature, float humidity, int soilMoisture) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  if (isnan(temperature) || isnan(humidity)) {
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Sensor Error!");
  } else {
    // Hiển thị nhiệt độ
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Temperature:");
    display.setTextSize(1);
    display.setCursor(0, 10);
    display.print(temperature);
    display.print(" C");

    // Hiển thị độ ẩm không khí
    display.setTextSize(1);
    display.setCursor(0, 22);
    display.println("Humidity:");
    display.setTextSize(1);
    display.setCursor(0, 32);
    display.print(humidity);
    display.print(" %");
    
    // Hiển thị độ ẩm đất
    display.setTextSize(1);
    display.setCursor(0, 44);
    display.println("Soil Moisture:");
    display.setTextSize(1);
    display.setCursor(0, 54);
    display.print(soilMoisture);
    display.print(" %");
  }
  display.display();
}

// Thêm hàm tương thích ngược để không phải sửa tất cả các lệnh gọi hiện có
void displaySensorData(float temperature, float humidity) {
  displaySensorData(temperature, humidity, 0);
}