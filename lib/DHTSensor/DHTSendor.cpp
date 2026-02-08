#include "DHTSensor.h"
#include <DHT.h>
#include <MqttHelper.h>  // Include the MQTT helper for publishing data

#define DHTPIN 2     // Pin where the DHT11 is connected
#define DHTTYPE DHT11   // DHT 11
const float TEMPERATURE_CALIBRATION_OFFSET = -1.1;  // Calibration offset for temperature
const float HUMIDITY_CALIBRATION_OFFSET = 8.0;  // Calibration offset for humidity

DHT dht(DHTPIN, DHTTYPE);
static bool dhtInitialized = false;

void setupDHT() {
    Serial.println("Initializing DHT sensor...");
    dht.begin();
    delay(2000);  // Give DHT sensor time to initialize
    dhtInitialized = true;
    Serial.println("DHT sensor initialized on pin " + String(DHTPIN));
}

void readAndPublishDHTData() {
    if (!dhtInitialized) {
        Serial.println("DHT sensor not initialized yet!");
        return;
    }
    
    Serial.println("--- Reading DHT sensor ---");
    
    // Read temperature as Celsius (with retry)
    float temperature = dht.readTemperature();
    if (isnan(temperature)) {
        delay(100);
        temperature = dht.readTemperature();  // Retry once
    }
    
    // Read humidity (with retry)
    float humidity = dht.readHumidity();
    if (isnan(humidity)) {
        delay(100);
        humidity = dht.readHumidity();  // Retry once
    }
    
    // Check if any reads failed and exit early (to try again).
    if (isnan(temperature) || isnan(humidity)) {
        Serial.println("ERROR: Failed to read from DHT sensor after retry!");
        Serial.println("Check wiring and sensor connection.");
        return;
    }
    
    // Apply calibration offsets
    temperature += TEMPERATURE_CALIBRATION_OFFSET;
    humidity += HUMIDITY_CALIBRATION_OFFSET;

    // Print and publish temperature
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
    bool tempPublished = publishReadout("dht_temperature", String(temperature, 1));
    Serial.println(tempPublished ? "Temperature published to MQTT" : "Failed to publish temperature");

    // Print and publish humidity
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    bool humidityPublished = publishReadout("dht_humidity", String(humidity, 1));
    Serial.println(humidityPublished ? "Humidity published to MQTT" : "Failed to publish humidity");
    
    Serial.println("--- DHT reading complete ---");
}