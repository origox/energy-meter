#include <MqttHelper.h>
#include <WiFiNINA.h>
#include <WiFiClient.h>  // Changed from WiFiSSLClient for non-SSL connection
#include "WifiSetup.h"

WiFiClient wifiClient;  // Use WiFiClient for port 1883 (non-SSL)
PubSubClient client(wifiClient);

static unsigned long lastReconnectAttempt = 0;
const unsigned long reconnectInterval = 5000;  // Try reconnecting every 5 seconds
const int maxReconnectAttempts = 3;  // Max attempts per reconnection cycle
static int reconnectAttempts = 0;

bool reconnect() {
  // Non-blocking reconnection with timeout
  unsigned long now = millis();
  
  if (client.connected()) {
    reconnectAttempts = 0;  // Reset counter when connected
    return true;
  }
  
  // Check if enough time has passed since last attempt
  if (now - lastReconnectAttempt < reconnectInterval) {
    return false;
  }
  
  lastReconnectAttempt = now;
  
  // Limit reconnection attempts
  if (reconnectAttempts >= maxReconnectAttempts) {
    Serial.println("Max reconnect attempts reached, waiting before retry...");
    reconnectAttempts = 0;  // Reset for next cycle
    return false;
  }
  
  checkWiFiConnection();  // Ensure WiFi is connected first
  
  Serial.print("Attempting MQTT connection...");
  reconnectAttempts++;
  
  if (client.connect("ArduinoClient", SECRET_MQTT_USERNAME, SECRET_MQTT_PASSWORD)) {
    Serial.println("connected");
    reconnectAttempts = 0;
    return true;
  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.print(" (attempt ");
    Serial.print(reconnectAttempts);
    Serial.print("/");
    Serial.print(maxReconnectAttempts);
    Serial.println(")");
    return false;
  }
}

void setupMqtt() {
  client.setServer(SECRET_MQTT_SERVER, SECRET_MQTT_PORT);
  client.setBufferSize(512);  // Increase buffer size for larger P1 messages
  reconnect();
  // Only publish if connected
  if (client.connected()) {
    client.publish((String(SECRET_MQTT_TOPIC_PREFIX) + "status").c_str(), "online");
  }
}

bool publishReadout(const String& topic, const String& payload) {
  if (!client.connected()) {
    reconnect();
  }
  
  // Always call loop to maintain connection
  client.loop();
  
  if (!client.connected()) {
    return false;  // Can't publish if not connected
  }
  
  // Publish and check for success
  bool success = client.publish((String(SECRET_MQTT_TOPIC_PREFIX) + topic).c_str(), payload.c_str());
  
  if (!success) {
    Serial.print("Failed to publish to topic: ");
    Serial.println(topic);
  }
  
  return success;
}

void mqttLoop() {
  // Maintain MQTT connection
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
