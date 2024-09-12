#include <WiFi.h>                 // WiFi library for ESP32
#include <ESPAsyncWebServer.h>    // Async WebServer library
#include <M5Stack.h>              // Include M5Stack library (if using M5Stack, otherwise remove this line)

// Define the buzzer pin
const int buzzerPin = 25;       // Change this to the pin you are using

// Replace with your network credentials
const char* ssid = "ACT102722051029";        // Your Wi-Fi SSID
const char* password = "99356842";  // Your Wi-Fi password

// Define PWM properties
const int ledChannel = 0;       // PWM channel
const int resolution = 8;       // PWM resolution (8-bit, range 0-255)

// Create an AsyncWebServer object on port 80
AsyncWebServer server(80);

// Placeholder for dynamic values
String processor(const String& var) {
  // When {{RSSI}} is found in HTML, replace with Wi-Fi signal strength
  if (var == "RSSI") {
    return String(WiFi.RSSI());
  }
  // Add more dynamic values as needed
  return String();
}

const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>ESP32 Buzzer Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <script>
    function sendValues() {
      var freq = document.getElementById("freq").value;
      var vol = document.getElementById("vol").value;
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/set?freq=" + freq + "&vol=" + vol, true);
      xhr.send();
    }

    function updateValue(id, value) {
      document.getElementById(id + "_value").innerText = value;
    }
  </script>
</head>
<body>
  <h1>Buzzer Control</h1>
  
  <label for="freq">Frequency (Hz):</label>
  <input type="range" id="freq" name="freq" min="100" max="10000" value="1000" step="10" 
         oninput="updateValue('freq', this.value)" onchange="sendValues()">
  <span id="freq_value">1000</span> Hz<br><br>

  <label for="vol">Volume (0-255):</label>
  <input type="range" id="vol" name="vol" min="0" max="255" value="0" step="1" 
         oninput="updateValue('vol', this.value)" onchange="sendValues()">
  <span id="vol_value">0</span><br><br>
  
</body>
</html>
)rawliteral";



void setup() {

  M5.begin();

  // Start Serial Monitor
  Serial.begin(115200);

  // Set up the PWM properties
  ledcSetup(ledChannel, 1000, resolution);
  ledcAttachPin(buzzerPin, ledChannel);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to Wi-Fi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Wi-Fi connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Serve the HTML page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", webpage);
  });

  // Handle requests to set frequency and volume
  server.on("/set", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("freq") && request->hasParam("vol")) {
      int freq = request->getParam("freq")->value().toInt();
      int vol = request->getParam("vol")->value().toInt();
      ledcWriteTone(ledChannel, freq); // Set frequency
      ledcWrite(ledChannel, vol);      // Set volume (duty cycle)
      request->send(200, "text/plain", "Buzzer set to frequency " + String(freq) + " Hz and volume " + String(vol));
    } else {
      request->send(400, "text/plain", "Bad Request");
    }
  });

  // Start server
  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  // Nothing to do here since it's asynchronous
}
