#include "SystemInfo.h"
#include <MqttHelper.h>
#include <Arduino_LSM6DS3.h>  // Include the library for the LSM6DS3 sensor

void printSystemInfo() {
    // Print IP Address
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
    publishReadout("ip_address", ip.toString());

    // Print MAC Address
    byte mac[6];
    WiFi.macAddress(mac);
    Serial.print("MAC Address: ");
    String macStr = "";
    for (int i = 0; i < 6; i++) {
        if (i > 0) {
            macStr += ":";
        }
        macStr += String(mac[i], HEX);
    }
    Serial.println(macStr);
    publishReadout("mac_address", macStr);
}

void readAndPublishSystemInfo() {
    // Initialize the LSM6DS3 sensor
    if (!IMU.begin()) {
        Serial.println("Failed to initialize LSM6DS3 sensor!");
        return;
    }
    // Read and publish temperature
    float temperature;
    if(IMU.temperatureAvailable()) {
        IMU.readTemperature(temperature);
        publishReadout("temperature", String(temperature));  // Publish temperature via MQTT
    }

    // Print RSSI (WiFi Signal Strength)
    long rssi = WiFi.RSSI();
    publishReadout("rssi", String(rssi));
}