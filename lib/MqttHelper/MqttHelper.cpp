#include <MqttHelper.h>
#include <WiFiNINA.h>
#include <WiFiClient.h>  // Changed from WiFiSSLClient for non-SSL connection
#include <Adafruit_SleepyDog.h>
#include "WifiSetup.h"

WiFiClient wifiClient;  // Use WiFiClient for port 1883 (non-SSL)
PubSubClient client(wifiClient);

static unsigned long lastReconnectAttempt = 0;
const unsigned long reconnectInterval = 5000;  // Try reconnecting every 5 seconds

unsigned long mqttReconnectCount = 0;

bool reconnect() {
  if (client.connected()) {
    return true;
  }

  // Non-blocking: only attempt once per interval
  unsigned long now = millis();
  if (now - lastReconnectAttempt < reconnectInterval) {
    return false;
  }
  lastReconnectAttempt = now;

  if (!checkWiFiConnection()) {
    return false;
  }

  Serial.print("Attempting MQTT connection...");
  Watchdog.reset();
  wifiClient.stop();  // Drop any half-open socket left by a silent disconnect

  if (client.connect("ArduinoClient", SECRET_MQTT_USERNAME, SECRET_MQTT_PASSWORD)) {
    Serial.println("connected");
    mqttReconnectCount++;
    client.publish((String(SECRET_MQTT_TOPIC_PREFIX) + "status").c_str(), "online");
    return true;
  }

  Serial.print("failed, rc=");
  Serial.println(client.state());
  return false;
}

void setupMqtt() {
  client.setServer(SECRET_MQTT_SERVER, SECRET_MQTT_PORT);
  client.setBufferSize(512);  // Increase buffer size for larger P1 messages
  client.setSocketTimeout(5);  // Default 15 s CONNACK wait would trip the watchdog
  reconnect();
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

bool mqttConnected() {
  return client.connected();
}

void mqttLoop() {
  // Maintain MQTT connection
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
