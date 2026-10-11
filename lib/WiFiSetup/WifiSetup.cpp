#include "WiFiSetup.h"
#include <Adafruit_SleepyDog.h>

unsigned long wifiReconnectCount = 0;

static void feedWatchdog() {
  Watchdog.reset();
}

void connectToWiFi() {
  wifiReconnectCount++;

  // WiFi.end() holds the NINA module in reset; begin() then reboots it.
  // Calling begin() alone cannot recover a wedged module.
  WiFi.end();
  Watchdog.reset();

  // begin() blocks up to _timeout (library default 50 s); keep it short and
  // let it feed the watchdog while it waits.
  WiFi.setTimeout(10000);
  WiFi.setFeedWatchdogFunc(feedWatchdog);

  Serial.print("Connecting to SSID: ");
  Serial.println(SECRET_SSID);
  WiFi.begin(SECRET_SSID, SECRET_PASS);
  Watchdog.reset();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi connected, IP: ");
    Serial.print(WiFi.localIP());
    Serial.print(" RSSI: ");
    Serial.println(WiFi.RSSI());
  } else {
    Serial.print("WiFi connection failed, status: ");
    Serial.println(WiFi.status());
  }
}

bool checkWiFiConnection() {
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }
  Serial.println("WiFi disconnected. Attempting to reconnect...");
  connectToWiFi();
  return WiFi.status() == WL_CONNECTED;
}
