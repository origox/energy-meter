#include <ObisParser.h>
#include <MqttHelper.h>

struct ObisCode {
  const char* code;
  const char* topic;
  const char* unit;
};

const ObisCode obisCodes[] = {
  {"0-0:1.0.0(", "timestamp", ""},
  {"1-0:1.8.1(", "total_energy_tariff1", " kWh"},
  {"1-0:1.8.2(", "total_energy_tariff2", " kWh"},
  {"1-0:2.8.1(", "energy_returned_tariff1", " kWh"},
  {"1-0:2.8.2(", "energy_returned_tariff2", " kWh"},
  {"1-0:1.7.0(", "current_power_usage", " kW"},
  {"1-0:2.7.0(", "current_power_returned", " kW"}
};

String extractValue(const String& line, char endChar) {
  int start = line.indexOf('(') + 1;
  int end = line.indexOf(endChar, start);
  return (start != -1 && end != -1) ? line.substring(start, end) : "";
}

void parseAndPublishData(const String& line) {
  for (const ObisCode& obis : obisCodes) {
    if (line.indexOf(obis.code) >= 0) {
      String value = extractValue(line, obis.unit[0] == ' ' ? '*' : 'S');
      if (!value.isEmpty()) {
        publishReadout(obis.topic, value + obis.unit);
        Serial.println("Published: Topic: " + String(obis.topic) + " Payload: " + value + obis.unit);
      }
      return;
    }
  }
  Serial.println("No recognized OBIS code found.");
}
