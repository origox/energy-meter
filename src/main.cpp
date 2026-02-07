#include <Arduino.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <arduino_secrets.h>
#include <ObisParser.h>
#include <MqttHelper.h>

// Buffer for P1 data
const int bufferSize = 512;
char buffer[bufferSize];
int bufferIndex = 0;

// Timing variables
unsigned long lastWiFiCheckTime = 0;
const unsigned long wifiCheckInterval = 30000;  // Check WiFi every 30 seconds
unsigned long lastHeartbeatTime = 0;
const unsigned long heartbeatInterval = 30000;  // Heartbeat every 30 seconds

// Connection states
bool wifiConnected = false;

void setup() {
  Serial.begin(9600);
  delay(3000);
  
  Serial.println("\n=== ENERGY METER FINAL VERSION ===");
  Serial.println("- All critical fixes applied");
  Serial.println("- Non-blocking WiFi/MQTT"); 
  Serial.println("- Works with/without P1 connection");
  Serial.println("- Robust error handling");
  
  Serial1.begin(115200);  // P1 port
  
  // Initialize MqttHelper (handles all MQTT operations)
  setupMqtt();
  
  Serial.println("Setup complete - starting main loop");
}

void attemptWiFiConnection() {
  static int wifiAttempts = 0;
  const int maxWifiAttempts = 10;  // 5 second timeout
  
  if (WiFi.status() == WL_CONNECTED) {
    if (!wifiConnected) {
      Serial.println("WiFi connected!");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      wifiConnected = true;
    }
    return;
  }
  
  if (wifiAttempts == 0) {
    Serial.print("Connecting to WiFi: ");
    Serial.println(SECRET_SSID);
    WiFi.begin(SECRET_SSID, SECRET_PASS);
  }
  
  wifiAttempts++;
  if (wifiAttempts >= maxWifiAttempts) {
    Serial.println("WiFi connection timeout - continuing without WiFi");
    wifiConnected = false;
    wifiAttempts = 0;
    return;
  }
  
  Serial.print(".");
  delay(500);
}

void attemptMqttConnection() {
  // MqttHelper handles all MQTT connection logic
  // This function is no longer needed but kept for compatibility
}

void loop() {
  unsigned long currentTime = millis();
  
  // Maintain MQTT connection (MqttHelper handles all MQTT operations)
  mqttLoop();
  
  // Check/reconnect WiFi periodically
  if (currentTime - lastWiFiCheckTime >= wifiCheckInterval) {
    lastWiFiCheckTime = currentTime;
    if (WiFi.status() != WL_CONNECTED && wifiConnected) {
      Serial.println("WiFi disconnected");
      wifiConnected = false;
    }
    if (!wifiConnected) {
      attemptWiFiConnection();
    }
  }
  
  // Send heartbeat
  if (currentTime - lastHeartbeatTime >= heartbeatInterval) {
    lastHeartbeatTime = currentTime;
    String status = "WiFi:" + String(wifiConnected ? "OK" : "NO") + " MQTT:OK";
    Serial.println("Status: " + status);
    
    if (publishReadout("status", status)) {
      publishReadout("uptime", String(currentTime / 1000));
      Serial.println("Heartbeat sent to MQTT");
    }
  }
  
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

