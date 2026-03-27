#include <WiFi.h>
#include <WebServer.h>
#include "index_html.h"
#define TX_PIN 4 // Wire this to the Video Board's RX pin
#define RX_PIN 5

const char* ssid = "COMPOSITE_SYNTH_UPLINK";
WebServer server(80);

// The Cyberpunk HTML & CSS Interface


void setup() {
  // Start the laptop debugging Serial (USB CDC)
  Serial.begin(115200);
  delay(3000); // Crucial for the C3: give the native USB time to wake up
  Serial1.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
  Serial.println("\n--- C3 Web Server Booting ---");
  WiFi.disconnect(true); 
  delay(100);
  // Start the Wi-Fi
  WiFi.mode(WIFI_AP);
  
  // --- ADD THIS LINE TO STOP THE CRASH ---
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  WiFi.softAP(ssid);
  // This tells the server to load your UI when someone visits the main IP
  server.on("/", []() {
    server.send(200, "text/html", htmlPage);
  });
  server.on("/updateState", []() {
    if (server.hasArg("data")) {
      String payload = server.arg("data");
      
      // Decode the URL encoding back into normal text spaces
      // payload.replace("%20", " ");
      
      // 1. Blast it down the physical wire (Pin 4)
      Serial1.println(payload); 
      
      // 2. Print it to the laptop so you can verify it works
      Serial.println(payload); 
    
    }
    server.send(200, "text/plain", "OK");
  });
  server.begin();
  Serial.println("Server online. Broadcasting Wi-Fi...");
}

void loop() {
  server.handleClient();
}