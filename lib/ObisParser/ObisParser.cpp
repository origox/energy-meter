#include <ObisParser.h>
#include <MqttHelper.h>

struct ObisCode {
  const char* code;
  const char* topic;
  const char* unit;
};

const ObisCode obisCodes[] = {
  {"0-0:1.0.0(", "timestamp", ""},
  {"1-0:1.8.0(", "total_energy_usage", " kWh"},
  {"1-0:2.8.0(", "total_energy_returned", " kWh"},
  {"1-0:3.8.0(", "reactive_energy_usage", " kVAr"},
  {"1-0:4.8.0(", "reactive_energy_returned", " kVAr"},
  {"1-0:1.7.0(", "current_power_usage", " kW"},
  {"1-0:2.7.0(", "current_power_returned", " kW"},
  {"1-0:3.7.0(", "current_reactive_usage", " kVAr"},
  {"1-0:4.7.0(", "current_reactive_returned", " kVAr"},
  {"1-0:21.7.0(", "current_energy_usage_phase1", " kWh"},
  {"1-0:41.7.0(", "current_energy_usage_phase2", " kWh"},
  {"1-0:61.7.0(", "current_energy_usage_phase2", " kWh"},  
  {"1-0:22.7.0(", "current_energy_returned_phase1", " kWh"},
  {"1-0:42.7.0(", "current_energy_returned_phase2", " kWh"},
  {"1-0:62.7.0(", "current_energy_returned_phase3", " kWh"},
  {"1-0:23.7.0(", "current_reactive_usage_phase1", " kVAr"},
  {"1-0:43.7.0(", "current_reactive_usage_phase2", " kVAr"},
  {"1-0:63.7.0(", "current_reactive_usage_phase3", " kVAr"},
  {"1-0:24.7.0(", "current_reactive_returned_phase1", " kVAr"},
  {"1-0:44.7.0(", "current_reactive_returned_phase2", " kVAr"},
  {"1-0:64.7.0(", "current_reactive_returned_phase3", " kVAr"},
  {"1-0:32.7.0(", "voltage_phase1", " V"},
  {"1-0:52.7.0(", "voltage_phase2", " V"},
  {"1-0:72.7.0(", "voltage_phase3", " V"},
  {"1-0:31.7.0(", "current_phase1", " A"},
  {"1-0:51.7.0(", "current_phase2", " A"},
  {"1-0:71.7.0(", "current_phase3", " A"}
};

const ObisCode* getObisCodeInfo(const String& code) {
    for (const auto& obis : obisCodes) {
        if (code.startsWith(obis.code)) {
            return &obis;
        }
    }
    return nullptr;
}

String extractValue(const String& line, char endChar) {
  int start = line.indexOf('(') + 1;
  int end = line.indexOf(endChar, start);
  return (start != -1 && end != -1) ? line.substring(start, end) : "";
}

void parseAndPublishData(const String& line) {
<<<<<<< HEAD
    const ObisCode* obis = getObisCodeInfo(line);
    if (obis) {
        String value = extractValue(line, obis->unit[0] == ' ' ? '*' : 'W');
        //Serial.println(obis->topic + ": " + value + obis->unit);
        publishReadout(obis->topic, value + obis->unit);
    }
}

// void parseAndPublishData(const String& line) {
//   // publishReadout("raw_data", line);
//   for (const ObisCode& obis : obisCodes) {
//     if (line.indexOf(obis.code) >= 0) {
//       String value = extractValue(line, obis.unit[0] == ' ' ? '*' : 'S');
//       if (!value.isEmpty()) {
//         publishReadout(obis.topic, value + obis.unit);
//         Serial.println("Published: Topic: " + String(obis.topic) + " Payload: " + value + obis.unit);
//       }
//       return;
//     }
//   }
//   Serial.println("No recognized OBIS code found.");
//   publishReadout("unknown", line);
// }
=======
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
>>>>>>> 373819b0fea4934a474927bb43f7b7e0dbea264d
