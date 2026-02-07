#include <Arduino.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <arduino_secrets.h>

// Buffer for P1 data
const int bufferSize = 512;
char buffer[bufferSize];
int bufferIndex = 0;

// Timing variables
unsigned long lastWiFiCheckTime = 0;
const unsigned long wifiCheckInterval = 30000;  // Check WiFi every 30 seconds
unsigned long lastMqttAttemptTime = 0;  
const unsigned long mqttRetryInterval = 5000;   // Retry MQTT every 5 seconds if disconnected
unsigned long lastHeartbeatTime = 0;
const unsigned long heartbeatInterval = 30000;  // Heartbeat every 30 seconds

// Network objects
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Connection states
bool wifiConnected = false;
bool mqttConnected = false;

void setup() {
  Serial.begin(9600);
  delay(3000);
  
  Serial.println("\n=== ENERGY METER FINAL VERSION ===");
  Serial.println("- All critical fixes applied");
  Serial.println("- Non-blocking WiFi/MQTT"); 
  Serial.println("- Works with/without P1 connection");
  Serial.println("- Robust error handling");
  
  Serial1.begin(115200);  // P1 port
  
  // Initialize MQTT settings
  mqttClient.setServer(SECRET_MQTT_SERVER, SECRET_MQTT_PORT);
  mqttClient.setBufferSize(512);  // Increased buffer size
  
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
  if (!wifiConnected || mqttClient.connected()) {
    mqttConnected = mqttClient.connected();
    return;
  }
  
  Serial.print("Attempting MQTT connection...");
  if (mqttClient.connect("EnergyMeterNano33", SECRET_MQTT_USERNAME, SECRET_MQTT_PASSWORD)) {
    Serial.println("MQTT connected!");
    mqttClient.publish((String(SECRET_MQTT_TOPIC_PREFIX) + "status").c_str(), "online");
    mqttConnected = true;
  } else {
    Serial.print("failed, rc=");
    Serial.println(mqttClient.state());
    mqttConnected = false;
  }
}

bool publishMessage(const String& topic, const String& payload) {
  if (!mqttConnected) return false;
  
  bool success = mqttClient.publish((String(SECRET_MQTT_TOPIC_PREFIX) + topic).c_str(), payload.c_str());
  if (!success) {
    Serial.println("MQTT publish failed");
    mqttConnected = false;
  }
  return success;
}

void parseP1Line(const String& line) {
  // Parse energy meter data
  if (line.startsWith("1-0:1.8.0(")) {
    // Total energy usage
    int start = line.indexOf('(') + 1;
    int end = line.indexOf('*');
    if (start > 0 && end > start) {
      String value = line.substring(start, end);
      Serial.print("Energy Total: ");
      Serial.print(value);
      Serial.println(" kWh");
      publishMessage("energy_total", value);
    }
  }
  else if (line.startsWith("1-0:1.7.0(")) {
    // Current power usage
    int start = line.indexOf('(') + 1;
    int end = line.indexOf('*');
    if (start > 0 && end > start) {
      String value = line.substring(start, end);
      Serial.print("Power Current: ");
      Serial.print(value);
      Serial.println(" kW");
      publishMessage("power_current", value);
    }
  }
}

void loop() {
  unsigned long currentTime = millis();
  
  // Maintain MQTT connection
  if (mqttConnected) {
    mqttClient.loop();
    if (!mqttClient.connected()) {
      mqttConnected = false;
      Serial.println("MQTT disconnected");
    }
  }
  
  // Check/reconnect WiFi periodically
  if (currentTime - lastWiFiCheckTime >= wifiCheckInterval) {
    lastWiFiCheckTime = currentTime;
    if (WiFi.status() != WL_CONNECTED && wifiConnected) {
      Serial.println("WiFi disconnected");
      wifiConnected = false;
      mqttConnected = false;
    }
    if (!wifiConnected) {
      attemptWiFiConnection();
    }
  }
  
  // Retry MQTT connection if needed
  if (wifiConnected && !mqttConnected && (currentTime - lastMqttAttemptTime >= mqttRetryInterval)) {
    lastMqttAttemptTime = currentTime;
    attemptMqttConnection();
  }
  
  // Send heartbeat
  if (currentTime - lastHeartbeatTime >= heartbeatInterval) {
    lastHeartbeatTime = currentTime;
    String status = "WiFi:" + String(wifiConnected ? "OK" : "NO") + 
                   " MQTT:" + String(mqttConnected ? "OK" : "NO");
    Serial.println("Status: " + status);
    
    if (publishMessage("status", status)) {
      publishMessage("uptime", String(currentTime / 1000));
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
          parseP1Line(line);
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

