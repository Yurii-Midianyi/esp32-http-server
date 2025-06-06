#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <uri/UriBraces.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6

WebServer server(80);

const int RED_LED = 26;
const int YELLOW_LED = 27;
const int GREEN_LED = 25;

unsigned long previousMillis = 0;
int currentLED = 0; // 0 = red, 1 = green, 2 = yellow

// Default durations (in milliseconds)
int redDuration = 3000;
int greenDuration = 3000;
int yellowDuration = 3000;

void switchLED() {
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);

  switch (currentLED) {
    case 0:
      digitalWrite(RED_LED, HIGH);
      currentLED = 1;
      previousMillis = millis();
      break;
    case 1:
      digitalWrite(GREEN_LED, HIGH);
      currentLED = 2;
      previousMillis = millis();
      break;
    case 2:
      digitalWrite(YELLOW_LED, HIGH);
      currentLED = 0;
      previousMillis = millis();
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
        input[type=number] { padding: 10px; font-size: 1.2em; width: 80px; }
        input[type=submit] { padding: 10px 20px; font-size: 1.2em; background: #28a745; color: white; border: none; margin-top: 10px; }
        .info { margin-top: 20px; font-size: 1.2em; }
      </style>
    </head>
    <body>
      <h1>ESP32 Traffic Light Control</h1>
      <form action="/setDurations" method="GET">
        <label>Red (sec): <input type="number" name="red" min="1" required value="REDVAL"></label><br><br>
        <label>Green (sec): <input type="number" name="green" min="1" required value="GREENVAL"></label><br><br>
        <label>Yellow (sec): <input type="number" name="yellow" min="1" required value="YELLOWVAL"></label><br><br>
        <input type="submit" value="Update Durations">
      </form>
      <div class="info">
        Current - Red: REDDUR s | Green: GREENDUR s | Yellow: YELLOWDUR s
      </div>
    </body>
    </html>
  )";

  html.replace("REDVAL", String(redDuration / 1000));
  html.replace("GREENVAL", String(greenDuration / 1000));
  html.replace("YELLOWVAL", String(yellowDuration / 1000));

  html.replace("REDDUR", String(redDuration / 1000));
  html.replace("GREENDUR", String(greenDuration / 1000));
  html.replace("YELLOWDUR", String(yellowDuration / 1000));

  server.send(200, "text/html", html);
}

void handleSetDurations() {
  if (server.hasArg("red")) {
    int val = server.arg("red").toInt();
    if (val > 0) redDuration = val * 1000;
  }
  if (server.hasArg("green")) {
    int val = server.arg("green").toInt();
    if (val > 0) greenDuration = val * 1000;
  }
  if (server.hasArg("yellow")) {
    int val = server.arg("yellow").toInt();
    if (val > 0) yellowDuration = val * 1000;
  }

  Serial.println("Updated durations:");
  Serial.print("Red: "); Serial.println(redDuration);
  Serial.print("Green: "); Serial.println(greenDuration);
  Serial.print("Yellow: "); Serial.println(yellowDuration);

  server.sendHeader("Location", "/");
  server.send(303); // redirect
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
  server.on("/setDurations", handleSetDurations);

  server.begin();
  Serial.println("HTTP server started (http://localhost:8180)");

  switchLED(); // Turn on the first light
}

void loop(void) {
  server.handleClient();

  unsigned long currentMillis = millis();
  int currentDuration = 0;

  switch (currentLED) {
    case 0: currentDuration = redDuration; break;
    case 1: currentDuration = greenDuration; break;
    case 2: currentDuration = yellowDuration; break;
  }

  if (currentMillis - previousMillis >= currentDuration) {
    switchLED();
  }
}
