#ifndef MQTT_HELPER_H
#define MQTT_HELPER_H

#include <PubSubClient.h>
#include <arduino_secrets.h>

void setupMqtt();
void publishReadout(const String& topic, const String& payload);

#endif // MQTT_HELPER_H
