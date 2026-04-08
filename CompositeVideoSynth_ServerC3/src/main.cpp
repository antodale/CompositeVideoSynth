#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h> // The ESP32 Non-Volatile Memory library!
#include "index_html.h"

#define TX_PIN 4  // Wire this to the Video Board's RX pin
#define RX_PIN 5

const char* ssid = "COMPOSITE_SYNTH_UPLINK";
WebServer server(80);
Preferences preferences; // Create the memory object

void setup() {
  // Start the laptop debugging Serial (USB CDC)
  Serial.begin(115200);
  delay(3000);  // Crucial for the C3: give the native USB time to wake up
  Serial1.begin(460800, SERIAL_8N1, RX_PIN, TX_PIN);
  
  Serial.println("\n--- C3 Web Server Booting ---");
  
  // Initialize the memory namespace called "synth" in read/write mode (false)
  preferences.begin("synth", false); 

  WiFi.disconnect(true);
  delay(100);
  
  // Start the Wi-Fi
  WiFi.mode(WIFI_AP);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  WiFi.softAP(ssid);

  // Serve the UI
  server.on("/", []() {
    server.send(200, "text/html", htmlPage);
  });

  // Handle live slider updates
  server.on("/updateState", []() {
    if (server.hasArg("data")) {
      String payload = server.arg("data");
      payload.replace("%20", " ");
      payload.replace("\r", "");   // Strip invisible carriage returns
      payload.replace("\n", "|");  // Disguise newlines as a pipe symbol!

      Serial.println("LIVE: " + payload); 
      Serial1.println(payload); // Blast it down the wire
    }
    server.send(200, "text/plain", "OK");
  });

  // --- NEW: Save Preset to Memory ---
  server.on("/savePreset", []() {
    if (server.hasArg("slot") && server.hasArg("data")) {
      String slot = server.arg("slot");
      String payload = server.arg("data");
      
      payload.replace("%20", " ");
      payload.replace("\r", "");
      payload.replace("\n", "|");

      // Save it to flash memory using the slot number as the key (e.g., "p1", "p2")
      String key = "p" + slot;
      preferences.putString(key.c_str(), payload);
      
      Serial.println("SAVED TO FLASH [" + key + "]: " + payload);
      server.send(200, "text/plain", "SAVED");
    } else {
      server.send(400, "text/plain", "ERROR");
    }
  });

  // --- NEW: Load Preset from Memory ---
  server.on("/loadPreset", []() {
    if (server.hasArg("slot")) {
      String slot = server.arg("slot");
      String key = "p" + slot;
      
      // Retrieve from flash memory. If the slot is empty, load a safe default.
      String payload = preferences.getString(key.c_str(), "A127064100032090");
      
      Serial.println("LOADED FROM FLASH [" + key + "]: " + payload);
      Serial1.println(payload); // Blast the saved preset to the video board!
      
      server.send(200, "text/plain", "LOADED");
    } else {
      server.send(400, "text/plain", "ERROR");
    }
  });

  server.begin();
  Serial.println("Server online. Broadcasting Wi-Fi...");
}

void loop() {
  server.handleClient();
}