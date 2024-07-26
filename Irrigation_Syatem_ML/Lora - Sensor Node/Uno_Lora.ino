#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <SPI.h>
#include <LoRa.h>

#define OLED_RESET 4
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define DHTPIN 4
#define DHTTYPE DHT11

int soilPin = A0;
int uvSensorPin = A1; // Change to A1 for correct pin reference
int mq135Pin = A2; // Change to A4 for correct pin reference
int mq2Pin = A3; // Change to A2 for correct pin reference
int rainSensorPin = A4; // Analog pin for rain sensor

DHT dht(DHTPIN, DHTTYPE);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);
  pinMode(soilPin, INPUT);
  pinMode(uvSensorPin, INPUT);
  pinMode(mq135Pin, INPUT);
  pinMode(mq2Pin, INPUT);
  pinMode(rainSensorPin, INPUT);
  dht.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println(F("Arduino Uno"));

  display.display();

  delay(2000);

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  int soilMoisture = map(analogRead(soilPin), 1023, 0, 0, 100);
  int uvValue = analogRead(uvSensorPin);
  int mq135Value = analogRead(mq135Pin);
  int mq2Value = analogRead(mq2Pin);
  int rainSensorValue = map(analogRead(rainSensorPin), 0, 1023, 0, 100); // Convert analog reading to percentage

  displaySensorData(temperature, humidity, soilMoisture, uvValue, mq135Value, mq2Value, rainSensorValue);

  Serial.println("Temperature: " + String(temperature) + "C");
  Serial.println("Humidity: " + String(humidity) + "%");
  Serial.println("Soil Moisture: " + String(soilMoisture) + "%");
  Serial.println("UV Sensor: " + String(uvValue) + " / " + String(map(uvValue, 0, 1023, 0, 100)) + "%");
  Serial.println("Air Quality:" + String(mq135Value) + " / " + String(map(mq135Value, 200, 800, 0, 100)) + "%");
  Serial.println("MQ-2 (Co2): " + String(mq2Value) + " / " + String(map(mq2Value, 0, 1023, 0, 100)) + "%");
  Serial.println("Rain Sensor: " + String(rainSensorValue) + "%");
  Serial.println();

  LoRa.beginPacket();
  // LoRa.print("Temperature: ");
  LoRa.print(temperature);
  LoRa.print(" / ");
  // LoRa.print("C, Humidity: ");
  LoRa.print(humidity);
  LoRa.print(" / ");
  // LoRa.print("%, Soil Moisture: ");
  LoRa.print(soilMoisture);
  LoRa.print(" / ");
  // LoRa.print("%, UV Sensor: ");
  LoRa.print(uvValue);
  LoRa.print(" / ");
  // LoRa.print(map(uvValue, 0, 1023, 0, 100));
  // LoRa.print("%, Air Quality: ");
  LoRa.print(mq135Value);
  LoRa.print(" / ");
  // LoRa.print(map(mq135Value, 200, 800, 0, 100));
  // LoRa.print("%, MQ-2 (Co2): ");
  LoRa.print(mq2Value);
  LoRa.print(" / ");
  // LoRa.print(map(mq2Value, 0, 1023, 0, 100));
  // LoRa.print("%, Rain Sensor: ");
  LoRa.print(rainSensorValue);
  LoRa.print(" / ");

  LoRa.endPacket();

  if (LoRa.endPacket()) {
    Serial.println("Data is Sent Successfully in LoRa");
  }

  delay(5000);
}

void displaySensorData(float temperature, float humidity, int soilMoisture, int uvValue, int mq135Value, int mq2Value, int rainSensorValue) {
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Temperature: " + String(temperature) + "C");
  display.println("Humidity: " + String(humidity) + "%");

  display.println();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 17);
  display.println("Soil Moisture: " + String(soilMoisture) + "%");

  display.println();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 27);
  display.println("UV Sensor: " + String(uvValue) + " / " + String(map(uvValue, 0, 1023, 0, 100)) + "%");

  display.println();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 37);
  display.println("Air Quality:" + String(mq135Value) + " / " + String(map(mq135Value, 200, 800, 0, 100)) + "%");

  display.println();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 47);
  display.println("MQ-2 (Co2): " + String(mq2Value) + " / " + String(map(mq2Value, 0, 1023, 0, 100)) + "%" );

  display.println();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 57);
  display.println("Rain Sensor: " + String(rainSensorValue) + "%");

  display.display();
}
