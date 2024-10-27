#include <Arduino.h>
#include <WiFiSetup.h>
#include <MqttHelper.h>
#include <ObisParser.h>

void setup() {
  Serial.begin(9600);
  Serial1.begin(115200);

  connectToWiFi();
  setupMqtt();
}

void loop() {
  if (Serial1.available() > 0) {
    String line = Serial1.readStringUntil('\n');
    line.trim();  // Remove any whitespace or newline characters
    Serial.println("Raw data: " + line);

    parseAndPublishData(line);  // Process and publish parsed data
  }
  delay(1000); 
}