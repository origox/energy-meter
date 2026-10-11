#include <Arduino.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <arduino_secrets.h>
#include <ObisParser.h>
#include <MqttHelper.h>
#include <WifiSetup.h>
#include <Adafruit_SleepyDog.h>
// #include <DHTSensor.h>  // Disabled - no DHT sensor connected

// Buffer for P1 data
const int bufferSize = 512;
char buffer[bufferSize];
int bufferIndex = 0;

// Timing variables
unsigned long lastHeartbeatTime = 0;
const unsigned long heartbeatInterval = 30000;  // Heartbeat every 30 seconds
// unsigned long lastDHTReadTime = 0;  // Disabled - no DHT sensor
// const unsigned long dhtReadInterval = 15000;  // Read DHT sensor every 15 seconds (for testing)

// Reboot if no heartbeat has reached the broker for this long. Catches the
// case where the loop still runs but WiFi/MQTT never come back.
unsigned long lastHeartbeatOkTime = 0;
const unsigned long maxOfflineTime = 600000;  // 10 minutes

// Hardware watchdog: reboots the board if loop() stalls, e.g. a hang inside
// the WiFiNINA driver. 16 s is the SAMD21 maximum.
const int watchdogTimeoutMs = 16000;

uint8_t resetCause = 0;  // PM->RCAUSE: 0x01 power-on, 0x04 brownout (3.3 V), 0x10 reset pin, 0x20 watchdog, 0x40 software

void setup() {
  resetCause = Watchdog.resetCause();
  Watchdog.enable(watchdogTimeoutMs);

  Serial.begin(9600);
  delay(3000);
  
  Serial.println("\n=== ENERGY METER ===");
  Serial.print("Reset cause: 0x");
  Serial.println(resetCause, HEX);
  
  Serial1.begin(115200);  // P1 port
  
  // Initialize MqttHelper (handles all WiFi/MQTT operations)
  setupMqtt();
  
  // Initialize DHT sensor - DISABLED (no sensor connected)
  // setupDHT();
  // Serial.println("DHT22 sensor initialized");
  
  Serial.println("Setup complete - starting main loop");
}

void loop() {
  Watchdog.reset();
  unsigned long currentTime = millis();
  
  // Maintain WiFi + MQTT connection (MqttHelper handles reconnects)
  mqttLoop();
  
  // Send heartbeat
  if (currentTime - lastHeartbeatTime >= heartbeatInterval) {
    lastHeartbeatTime = currentTime;
    bool wifiOk = WiFi.status() == WL_CONNECTED;
    String status = "WiFi:" + String(wifiOk ? "OK" : "NO") + " MQTT:" + String(mqttConnected() ? "OK" : "NO");
    Serial.println("Status: " + status);
    
    if (publishReadout("status", status)) {
      lastHeartbeatOkTime = currentTime;
      publishReadout("uptime", String(currentTime / 1000));
      publishReadout("rssi", String(WiFi.RSSI()));
      publishReadout("wifi_reconnects", String(wifiReconnectCount));
      publishReadout("mqtt_reconnects", String(mqttReconnectCount));
      publishReadout("reset_cause", String(resetCause));
      Serial.println("Heartbeat sent to MQTT");
    }
  }
  
  if (currentTime - lastHeartbeatOkTime >= maxOfflineTime) {
    Serial.println("No heartbeat delivered for 10 minutes - rebooting");
    Serial.flush();
    NVIC_SystemReset();
  }
  
  // Read and publish DHT sensor data periodically - DISABLED (no sensor connected)
  /*
  if (currentTime - lastDHTReadTime >= dhtReadInterval) {
    lastDHTReadTime = currentTime;
    Serial.println("=== DHT SENSOR READ CYCLE ===");
    Serial.println("Time since last read: " + String(currentTime - lastDHTReadTime + dhtReadInterval) + " ms");
    readAndPublishDHTData();
  }
  */
  
  // Read P1 data
  while (Serial1.available() > 0) {
    char incomingByte = Serial1.read();
    
    if (incomingByte == '\n' || incomingByte == '\r') {
      if (bufferIndex > 0) {
        buffer[bufferIndex] = '\0';
        String line = String(buffer);
        line.trim();
        
        if (line.length() > 0) {
          Serial.print("P1: ");
          Serial.println(line);
          parseAndPublishData(line);  // Use comprehensive ObisParser
        }
        bufferIndex = 0;
      }
    } 
    else if (incomingByte >= 32 && incomingByte <= 126) {
      if (bufferIndex < bufferSize - 1) {
        buffer[bufferIndex++] = incomingByte;
      } else {
        Serial.println("Buffer overflow");
        bufferIndex = 0;
      }
    }
  }
  
  delay(10);  // Keep responsive
}

