#include <Arduino.h>
#include "WiFiSetup/WiFiSetup.h"

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);

  connectToWiFi();
}

void loop() {
  if (Serial1.available() > 0) {
    String line = Serial1.readStringUntil('\n');
    line.trim();  // Remove any whitespace or newline characters
    Serial.println("Raw data: " + line);

    //parseAndPublishData(line);  // Process and publish parsed data
  }
  delay(1000);
}