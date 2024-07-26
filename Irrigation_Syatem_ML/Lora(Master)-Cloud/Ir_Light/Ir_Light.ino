const int irSensorPin = D1;  // Pin for the IR sensor
const int relayPin = D2;     // Pin for the relay

void setup() {
  pinMode(irSensorPin, INPUT);
  pinMode(relayPin, OUTPUT);
}

void loop() {
  int sensorValue = digitalRead(irSensorPin);
  
  if (sensorValue == HIGH) {
    digitalWrite(relayPin, LOW);  // Turn on the relay
  } else {
    digitalWrite(relayPin, HIGH);   // Turn off the relay
  }

  delay(100);  // Delay for stability
}
