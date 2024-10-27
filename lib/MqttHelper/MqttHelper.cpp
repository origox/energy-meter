#include <MqttHelper.h>
#include <WiFiClient.h>

WiFiClient wifiClient;
PubSubClient client(wifiClient);

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ArduinoClient", SECRET_MQTT_USERNAME, SECRET_MQTT_PASSWORD)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds");
      delay(5000);
    }
  }
}

void setupMqtt() {
  client.setServer(SECRET_MQTT_SERVER, SECRET_MQTT_PORT);
  reconnect();
  client.publish("apa", "bepa");
}

void publishReadout(const String& topic, const String& payload) {
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); // Necessary to keep MQTT connection alive
  client.publish((String(SECRET_MQTT_TOPIC_PREFIX) + topic).c_str(), payload.c_str());
}
