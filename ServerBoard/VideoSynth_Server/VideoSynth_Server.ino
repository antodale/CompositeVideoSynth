#include <WiFi.h>
#include <WebServer.h>


const char* ssid = "RETRO_SYNTH_UPLINK";
WebServer server(80); // Changed from ESP8266WebServer

// The Cyberpunk HTML & CSS Interface
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <style>
    body { background-color: #000; color: #0f0; font-family: 'Courier New', monospace; text-align: center; margin-top: 15%; }
    h2 { font-size: 24px; text-shadow: 0 0 5px #0f0; }
    p { font-size: 14px; margin-bottom: 30px; }
    input[type=text] { background: #0a0a0a; border: 1px solid #0f0; color: #0f0; padding: 15px; font-family: 'Courier New', monospace; font-size: 18px; width: 80%; max-width: 300px; margin-bottom: 25px; outline: none; text-align: center; text-transform: uppercase; }
    input[type=text]:focus { box-shadow: 0 0 10px #0f0; }
    input[type=submit] { background: #0f0; color: #000; border: none; padding: 15px 30px; font-family: 'Courier New', monospace; font-size: 18px; font-weight: bold; cursor: pointer; text-transform: uppercase; }
    input[type=submit]:active { background: #fff; color: #000; }
  </style>
</head>
<body>
  <h2>[ UPLINK ESTABLISHED ]</h2>
  <p>ENTER TRANSMISSION DATA:</p>
  <form action="/update">
    <input type="text" name="text" maxlength="20" autocomplete="off" autofocus>
    <br>
    <input type="submit" value="TRANSMIT TO CRT">
  </form>
</body>
</html>
)rawliteral";

void setup() {
  // We use Serial1 (Hardware UART) and map it to our specific physical pins
  // Serial1.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
  
  // // Primary Serial for Laptop Debugging (Fixed the 11520 typo here!)
  // Serial.begin(115200);
  // Serial.println("Setup ...");
  
  WiFi.softAP(ssid);

  server.on("/", []() {
    server.send(200, "text/html", htmlPage);
  });

  server.on("/update", []() {
    if (server.hasArg("text")) {
      String newText = server.arg("text");
      newText.toUpperCase(); 
      
      // Blast it out the physical TX pin to the other board!
      // Serial1.println(newText); 
      
      // // Also print it to the laptop Serial Monitor so you can verify it worked
      // Serial.print("Uplink Transmitted: ");
      // Serial.println(newText);
    }
    // server.sendHeader("Location", "/");
    // server.send(302, "text/plain", "");
  });

  server.begin();
  Serial.println("Server online");
}

void loop() {
  server.handleClient();
}