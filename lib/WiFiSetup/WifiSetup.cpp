#include "WiFiSetup.h"

void connectToWiFi() {
  Serial.println("WiFi.begin() called");
  
  WiFi.begin(SECRET_SSID, SECRET_PASS);
  Serial.print("Connecting to SSID: ");
  Serial.println(SECRET_SSID);
  
  int attempts = 0;
  const int maxAttempts = 20; // 10 seconds timeout
  
  while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
    delay(500);
    Serial.print(".");
    attempts++;
    
    // Show status every 5 attempts
    if (attempts % 5 == 0) {
      Serial.print(" [Status: ");
      Serial.print(WiFi.status());
      Serial.print(", Attempt: ");
      Serial.print(attempts);
      Serial.println("]");
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi connection failed after timeout");
    Serial.print("WiFi status: ");
    Serial.println(WiFi.status());
    Serial.println("Continuing without WiFi...");
  }
}

void checkWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected. Attempting to reconnect...");
    connectToWiFi();
  }
}
