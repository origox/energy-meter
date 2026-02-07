#ifndef WIFI_SETUP_H
#define WIFI_SETUP_H

#include <SPI.h>
#include <WiFiNINA.h>
#include <arduino_secrets.h> // Include secrets file

void connectToWiFi();
void checkWiFiConnection();

#endif // WIFI_SETUP_H
