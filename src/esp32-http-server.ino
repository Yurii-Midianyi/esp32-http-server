#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <uri/UriBraces.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6

WebServer server(80);

// LED pins
const int RED_LED = 26;
const int YELLOW_LED = 27;
const int GREEN_LED = 25;

unsigned long previousMillis = 0;
int interval = 3000;  // default 3 seconds
int currentLED = 0;   // 0 = red, 1 = green, 2 = yellow

void switchLED() {
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);

  switch (currentLED) {
    case 0:
      digitalWrite(RED_LED, HIGH);
      currentLED = 1;
      break;
    case 1:
      digitalWrite(GREEN_LED, HIGH);
      currentLED = 2;
      break;
    case 2:
      digitalWrite(YELLOW_LED, HIGH);
      currentLED = 0;
      break;
  }
}

void handleRoot() {
  String html = R"(
    <!DOCTYPE html>
    <html>
    <head>
      <title>Traffic Light</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>
        body { font-family: Arial; text-align: center; margin-top: 50px; }
        input[type=number] { padding: 10px; font-size: 1.2em; }
        input[type=submit] { padding: 10px 20px; font-size: 1.2em; background: #28a745; color: white; border: none; }
        .info { margin-top: 20px; font-size: 1.2em; }
      </style>
    </head>
    <body>
      <h1>ESP32 Traffic Light Control</h1>
      <form action="/setInterval" method="GET">
        <label for="seconds">Set interval (seconds):</label><br><br>
        <input type="number" id="seconds" name="seconds" min="1" required>
        <input type="submit" value="Update Interval">
      </form>
      <div class="info">Current interval: INTERVAL seconds</div>
    </body>
    </html>
  )";

  html.replace("INTERVAL", String(interval / 1000));
  server.send(200, "text/html", html);
}

void handleSetInterval() {
  if (server.hasArg("seconds")) {
    int sec = server.arg("seconds").toInt();
    if (sec > 0) {
      interval = sec * 1000;
      Serial.print("Updated interval to: ");
      Serial.print(sec);
      Serial.println(" seconds");
    }
  }
  server.sendHeader("Location", "/");
  server.send(303);  // redirect
}

void setup(void) {
  Serial.begin(115200);

  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
  Serial.print("Connecting to WiFi ");
  Serial.print(WIFI_SSID);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println(" Connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/setInterval", handleSetInterval);

  server.begin();
  Serial.println("HTTP server started (http://localhost:8180)");
}

void loop(void) {
  server.handleClient();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    switchLED();
  }

  //delay(2); // small delay to yield CPU
}