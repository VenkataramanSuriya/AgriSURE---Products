#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Firebase_ESP_Client.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26
#define BAND 433E6

#define OLED_SDA 4
#define OLED_SCL 15 
#define OLED_RST 16
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// #define API_KEY "AIzaSyDrQhm6deJYZuqxoxT-4rRRdtQdgIYG-pQ"
#define API_KEY "AIzaSyCgV98yxzzBf00RA9CDxOAx3pzFQs_cM0c"
#define DATABASE_URL "https://sensor-a7cb1-default-rtdb.asia-southeast1.firebasedatabase.app"
// #define DATABASE_URL "smart-precision-agricult-2775f-default-rtdb.firebaseio.com/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

String LoRaData;

const char* ssid = "Galaxy F13";
const char* wifi_password = "Surya@4646";
const char* mqtt_server = "192.168.19.209";
const char* mqtt_topic = "Soilmoisture";
const char* clientID = "Zone_A_SoilSensor";
WiFiClient wifiClient;
PubSubClient client(wifiClient);

void setup() {
  Serial.begin(115200);

  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("LORA RECEIVER ");
  display.display();

  Serial.println("LoRa Receiver Test");

  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initializing OK!");
  display.setCursor(0,10);
  display.println("LoRa Initializing OK!");
  display.display();  

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);


  client.setServer(mqtt_server, 1883);

  if (client.connect(clientID)) {
    Serial.println("Connected to MQTT Broker!");
  }
  else {
    Serial.println("Connection to MQTT Broker failed...");
  }
}

void loop() {

  String soilMoistureStr, temperatureStr, humidityStr, uvStr, mq135Str, mq2Str, rainStr;
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.print("Received packet ");

    // Read LoRa data byte by byte
    while (LoRa.available()) {
      LoRaData = LoRa.readString();
      Serial.print(LoRaData);
      char c = LoRa.read();
      Serial.print(c);
      LoRaData += c; // Append the character to the LoRaData string
    }

    

    // Publish sensor values to MQTT broker
    // String message =  //soilMoistureStr+"#"+temperatureStr+"#"+humidityStr+"#"+uvStr+"#"+mq135Str+"#"+mq2Str+"#"+rainStr;
    // client.publish(mqtt_topic, message.c_str());
    String message = LoRaData;
    delay(5000);
    client.publish(mqtt_topic,message.c_str());
// Split the received data into sensor values
    int separatorIndex = LoRaData.indexOf('#');
    if (separatorIndex != -1) {
      String soilMoistureStr = LoRaData.substring(0, separatorIndex);
      LoRaData.remove(0, separatorIndex + 1);

      separatorIndex = LoRaData.indexOf('#');
      if (separatorIndex != -1) {
        String temperatureStr = LoRaData.substring(0, separatorIndex);
        LoRaData.remove(0, separatorIndex + 1);

        separatorIndex = LoRaData.indexOf('#');
        if (separatorIndex != -1) {
          String humidityStr = LoRaData.substring(0, separatorIndex);
          LoRaData.remove(0, separatorIndex + 1);

          separatorIndex = LoRaData.indexOf('#');
          if (separatorIndex != -1) {
            String uvStr = LoRaData.substring(0, separatorIndex);
            LoRaData.remove(0, separatorIndex + 1);

            separatorIndex = LoRaData.indexOf('#');
            if (separatorIndex != -1) {
              String mq135Str = LoRaData.substring(0, separatorIndex);
              LoRaData.remove(0, separatorIndex + 1);

              separatorIndex = LoRaData.indexOf('#');
              if (separatorIndex != -1) {
                String mq2Str = LoRaData.substring(0, separatorIndex);
                LoRaData.remove(0, separatorIndex + 1);

                String rainStr = LoRaData;

                // Display sensor values on the serial monitor
                Serial.println();
                Serial.println("Soil Moisture: " + soilMoistureStr);
                Serial.println("Temperature: " + temperatureStr);
                Serial.println("Humidity: " + humidityStr);
                Serial.println("UV: " + uvStr);
                Serial.println("MQ135: " + mq135Str);
                Serial.println("MQ2: " + mq2Str);
                Serial.println("Rain: " + rainStr);

                if (Firebase.RTDB.setString(&fbdo, "humidity", humidityStr.c_str())) {
                  Serial.print("Humidity: ");
                  Serial.println(humidityStr);
                  Serial.println("%");
                } else {
                  Serial.println("FAILED");
                  Serial.println("REASON: " + fbdo.errorReason());
                }

                if (Firebase.RTDB.setString(&fbdo, "temperature", temperatureStr.c_str())) {
                  Serial.print("Temperature: ");
                  Serial.println(temperatureStr);
                  Serial.println("°C");
                } else {
                  Serial.println("FAILED");
                  Serial.println("REASON: " + fbdo.errorReason());
                }

                if (Firebase.RTDB.setString(&fbdo, "SoilMoisture", soilMoistureStr.c_str())) {
                  Serial.print("Soil Moisture: ");
                  Serial.println(soilMoistureStr);
                  Serial.println("%");
                } else {
                  Serial.println("FAILED");
                  Serial.println("REASON: " + fbdo.errorReason());
                }

                if (Firebase.RTDB.setString(&fbdo, "UVSensor", uvStr.c_str())) {
                  Serial.print("UV Sensor: ");
                  Serial.println(uvStr);
                  Serial.println("%");
                } else {
                  Serial.println("FAILED");
                  Serial.println("REASON: " + fbdo.errorReason());
                }

                if (Firebase.RTDB.setString(&fbdo, "MQ135Sensor", mq135Str.c_str())) {
                  Serial.print("MQ135 Sensor: ");
                  Serial.println(mq135Str);
                  Serial.println("%");
                } else {
                  Serial.println("FAILED");
                  Serial.println("REASON: " + fbdo.errorReason());
                }

                if (Firebase.RTDB.setString(&fbdo, "MQ2Sensor", mq2Str.c_str())) {
                  Serial.print("MQ2 Sensor: ");
                  Serial.println(mq2Str);
                  Serial.println("%");
                  

                } else {
                  Serial.println("FAILED");
                  Serial.println("REASON: " + fbdo.errorReason());
                }
              }
            }
          }
        }
      }
    }
    int rssi = LoRa.packetRssi();
    Serial.print("RSSI: ");
    Serial.println(rssi);

    // Display data on OLED
    display.clearDisplay();
    display.setCursor(0,0);
    display.print("LORA RECEIVER");
    display.setCursor(0,20);
    display.print("Received packet:");
    display.setCursor(0,30);
    display.print(LoRaData);
    display.setCursor(0,40);
    display.display();

    // Reset LoRaData for next packet
    LoRaData = "";
  }
}
