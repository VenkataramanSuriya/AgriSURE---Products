#include <ESP8266WiFi.h> 
#include <PubSubClient.h> 
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

const int ledPin = D6; 
const char* ssid = "Galaxy F13";
const char* wifi_password = "Surya@4646";
const char* mqtt_server = "192.168.168.209";
const char* mqtt_topic = "waterpump";
const char* clientID = "Motor1";
char message_buff[100];

WiFiClient wifiClient;
PubSubClient client(mqtt_server, 1883, wifiClient); 00  

LiquidCrystal_I2C lcd(0x27, 20, 4); 

void ReceivedMessage(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  int i=0;
  for (i=0;i<length;i++) {
    Serial.print((char)payload[i]);
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';
  Serial.println();
  String motor = String(message_buff);
  Serial.println(motor);
  if (motor == "ON") {
    digitalWrite(ledPin, HIGH);
    lcd.setCursor(4, 2);
    lcd.print("MOTOR IS ON ");
    Serial.println("MOTOR IS ON");

    lcd.setCursor(2, 0);
    lcd.print("TECH VESIONARIES");
  }
  else{
    digitalWrite(ledPin, LOW);
    lcd.setCursor(4, 2);
    lcd.print("MOTOR IS OFF");
    Serial.println("MOTOR IS OFF");

    lcd.setCursor(2, 0);
    lcd.print("TECH VESIONARIES");
  }
}

bool Connect() {
  if (client.connect(clientID)) {
      client.subscribe(mqtt_topic);
      return true;
    }
    else {
      return false;
  }
}

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  Serial.begin(115200);

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

  client.setCallback(ReceivedMessage);
  if (Connect()) {
    Serial.println("Connected Successfully to MQTT Broker!");  
  }
  else {
    Serial.println("Connection Failed!");
  }

  lcd.init();
  lcd.backlight();
}

void loop() {
  if (!client.connected()) {
    Connect();
  }
  client.loop();
}
