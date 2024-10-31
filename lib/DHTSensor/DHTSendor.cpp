#include "DHTSensor.h"
#include <DHT.h>
#include <MqttHelper.h>  // Include the MQTT helper for publishing data

#define DHTPIN 2     // Pin where the DHT11 is connected
#define DHTTYPE DHT11   // DHT 11
const float TEMPERATURE_CALIBRATION_OFFSET = -1.1;  // Calibration offset for temperature
const float HUMIDITY_CALIBRATION_OFFSET = 8.0;  // Calibration offset for humidity

DHT dht(DHTPIN, DHTTYPE);

void setupDHT() {
    dht.begin();
}

void readAndPublishDHTData() {
    // Read temperature as Celsius
    float temperature = dht.readTemperature() + TEMPERATURE_CALIBRATION_OFFSET;
    // Read humidity
    float humidity = dht.readHumidity() + HUMIDITY_CALIBRATION_OFFSET;

    // Check if any reads failed and exit early (to try again).
    if (isnan(temperature) || isnan(humidity)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }

    // Print and publish temperature
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
    publishReadout("dht_temperature", String(temperature));

    // Print and publish humidity
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    publishReadout("dht_humidity", String(humidity));
}