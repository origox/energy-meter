#include <Arduino.h>
#include <WiFiSetup.h>
#include <MqttHelper.h>
#include <ObisParser.h>
#include <SystemInfo.h> 
#include <DHTSensor.h> 

const int bufferSize = 256;  // Define a buffer size
char buffer[bufferSize];     // Buffer to store incoming data
int bufferIndex = 0;         // Index to keep track of buffer position
unsigned long lastTempReadTime = 0;  // Last time temperature was read
const unsigned long tempReadInterval = 2000;  // Interval to read temperature (every 2 seconds)

void setup() {
  Serial.begin(9600);
  Serial1.begin(115200);

  connectToWiFi();
  setupMqtt();
  setupDHT();
  printSystemInfo();
}

void loop() {
  // Read and publish temperature periodically
  unsigned long currentTime = millis();
  if (currentTime - lastTempReadTime >= tempReadInterval) {
    lastTempReadTime = currentTime;
    readAndPublishSystemInfo();  // Read and publish temperature
    readAndPublishDHTData();  // Read and publish DHT data
  }

  while (Serial1.available() > 0) {
    char incomingByte = Serial1.read();
    if (incomingByte == '\n') {
      buffer[bufferIndex] = '\0';  // Null-terminate the string
      String line = String(buffer);
      line.trim();  // Remove any whitespace or newline characters
      Serial.println("Raw data: " + line);

      publishReadout("LINE: ", line);  // Publish raw data

      parseAndPublishData(line);  // Process and publish parsed data

      bufferIndex = 0;  // Reset buffer index for the next line
    } else {
      if (bufferIndex < bufferSize - 1) {
        buffer[bufferIndex++] = incomingByte;  // Add byte to buffer
      } else {
        // Buffer overflow, handle error
        Serial.println("Buffer overflow");
        bufferIndex = 0;  // Reset buffer index
      }
    }
  }
}