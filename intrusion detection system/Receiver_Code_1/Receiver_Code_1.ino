#include <ESP8266WiFi.h>
#include <espnow.h>

#define BUZZER_PIN D2

void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  String receivedMessage = "";
  for (int i = 0; i < len; i++) {
    receivedMessage += (char)incomingData[i];
  }
  if (receivedMessage == "ALARM") {
    Serial.println("Intrusion alarm triggered!");
    digitalWrite(BUZZER_PIN, HIGH);  // Activate the buzzer
    delay(5000);                    // Buzzer duration
    digitalWrite(BUZZER_PIN, LOW);   // Deactivate the buzzer
  }
}

void setup() {
  Serial.begin(74880);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);  // Ensure buzzer is off initially
}

void loop() {
  // Nothing to do here
}