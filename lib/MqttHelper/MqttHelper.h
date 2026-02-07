#ifndef MQTT_HELPER_H
#define MQTT_HELPER_H

#include <PubSubClient.h>
#include <arduino_secrets.h>

void setupMqtt();
void mqttLoop();  // Call this in main loop to maintain connection
bool publishReadout(const String& topic, const String& payload);  // Returns true if publish succeeded

#endif // MQTT_HELPER_H
