#include <ESP8266WiFi.h>
#include <espnow.h>

// Define the MAC addresses for the receivers
uint8_t receiverAddress1[] = { 0xE8, 0xDB, 0x84, 0xE7, 0x8A, 0x63 }; // Receiver 1
uint8_t receiverAddress2[] = { 0x50, 0x02, 0x91, 0xD2, 0x88, 0x9F }; // Receiver 2

// Define the GPIO pins for the PIR sensors
#define SENSOR_PIN1 D1
#define SENSOR_PIN2 D2

// Define the GPIO pin for the buzzer
#define BUZZER_PIN D3

// Define debounce time in milliseconds
#define DEBOUNCE_TIME 3000  // 3 seconds debounce time

// Variables to store the last debounce time
unsigned long lastDebounceTime1 = 0;
unsigned long lastDebounceTime2 = 0;

// Variables to store the debounce state
bool lastSensor1State = LOW;
bool lastSensor2State = LOW;

void setup() {
  Serial.begin(74880);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);

  // Add peers
  if (esp_now_add_peer(receiverAddress1, ESP_NOW_ROLE_SLAVE, 0, NULL, 0) != 0) {
    Serial.println("Failed to add peer 1");
    return;
  }

  if (esp_now_add_peer(receiverAddress2, ESP_NOW_ROLE_SLAVE, 0, NULL, 0) != 0) {
    Serial.println("Failed to add peer 2");
    return;
  }

  pinMode(SENSOR_PIN1, INPUT);
  pinMode(SENSOR_PIN2, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);  // Set buzzer pin as output
}

void loop() {
  bool currentSensor1State = digitalRead(SENSOR_PIN1);
  bool currentSensor2State = digitalRead(SENSOR_PIN2);

  unsigned long currentTime = millis();

  // Handle sensor 1 with debounce logic
  if (currentSensor1State != lastSensor1State) {
    lastDebounceTime1 = currentTime;
  }

  if ((currentTime - lastDebounceTime1) > DEBOUNCE_TIME) {
    if (currentSensor1State == HIGH) {
      Serial.println("Sensor 1 triggered! Intrusion detected!");
      String message = "ALARM";
      // Send message to all peers
      int result1 = esp_now_send(receiverAddress1, (uint8_t *)message.c_str(), message.length());
      int result2 = esp_now_send(receiverAddress2, (uint8_t *)message.c_str(), message.length());

      if (result1 == 0 && result2 == 0) {
        Serial.println("Alarm sent successfully to all peers");
      } else {
        Serial.println("Error sending alarm");
      }

      // Activate buzzer
      digitalWrite(BUZZER_PIN, HIGH);
      delay(1000); // Buzzer on for 1 second
      digitalWrite(BUZZER_PIN, LOW);
    }
  }

  // Handle sensor 2 with debounce logic
  if (currentSensor2State != lastSensor2State) {
    lastDebounceTime2 = currentTime;
  }

  if ((currentTime - lastDebounceTime2) > DEBOUNCE_TIME) {
    if (currentSensor2State == HIGH) {
      Serial.println("Sensor 2 triggered! Intrusion detected!");
      String message = "ALARM";
      // Send message to all peers
      int result1 = esp_now_send(receiverAddress1, (uint8_t *)message.c_str(), message.length());
      int result2 = esp_now_send(receiverAddress2, (uint8_t *)message.c_str(), message.length());

      if (result1 == 0 && result2 == 0) {
        Serial.println("Alarm sent successfully to all peers");
      } else {
        Serial.println("Error sending alarm");
      }

      // Activate buzzer
      digitalWrite(BUZZER_PIN, HIGH);
      delay(1000); // Buzzer on for 1 second
      digitalWrite(BUZZER_PIN, LOW);
    }
  }

  // Update the last state of sensors
  lastSensor1State = currentSensor1State;
  lastSensor2State = currentSensor2State;

  delay(2000); // Short delay to avoid excessive processing
}