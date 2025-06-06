#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6

WebServer server(80);

const int RED_LED = 26;
const int YELLOW_LED = 27;
const int GREEN_LED = 25;

unsigned long previousMillis = 0;
int currentLED = 0; // 0 = red, 1 = green, 2 = yellow

// Параметри алгоритму Вебстера
const float L = 5.0; // втрати часу (сек)
float qi[2] = {400.0, 600.0}; // інтенсивність трафіку для фаз (машин/год)
float si[2] = {1800.0, 1800.0}; // пропускна здатність (машин/год)

int redDuration = 3000;
int greenDuration = 3000;
int yellowDuration = 3000;

void calculateWebsterDurations() {
  float Yi[2];
  Yi[0] = qi[0] / si[0];
  Yi[1] = qi[1] / si[1];
  float Y = Yi[0] + Yi[1];

  // захист від ділення на нуль чи Y>=1
  if (Y >= 1.0) {
    Serial.println("Y >= 1. Не можна обчислити цикл. Використовується мінімальний цикл.");
    greenDuration = 5000;
    redDuration = 5000;
    yellowDuration = 3000;
    return;
  }

  float C = (1.5 * L + 5.0) / (1.0 - Y);
  float gi1 = (Yi[0] / Y) * (C - L);
  float gi2 = (Yi[1] / Y) * (C - L);

  greenDuration = (int)(gi1 * 1000);
  redDuration = (int)(gi2 * 1000);
  yellowDuration = 3000; // фіксовано

  Serial.println("Webster Algorithm Updated:");
  Serial.print("q1: "); Serial.println(qi[0]);
  Serial.print("s1: "); Serial.println(si[0]);
  Serial.print("q2: "); Serial.println(qi[1]);
  Serial.print("s2: "); Serial.println(si[1]);
  Serial.print("Green Duration: "); Serial.println(greenDuration);
  Serial.print("Red Duration: "); Serial.println(redDuration);
}

// REST API для введення трафіку
void handleSetTrafficData() {
  if (server.hasArg("q1")) qi[0] = server.arg("q1").toFloat();
  if (server.hasArg("s1")) si[0] = server.arg("s1").toFloat();
  if (server.hasArg("q2")) qi[1] = server.arg("q2").toFloat();
  if (server.hasArg("s2")) si[1] = server.arg("s2").toFloat();

  calculateWebsterDurations();

  String response = "Traffic data updated:\n";
  response += "q1=" + String(qi[0]) + " s1=" + String(si[0]) + "\n";
  response += "q2=" + String(qi[1]) + " s2=" + String(si[1]) + "\n";
  response += "New greenDuration=" + String(greenDuration/1000.0) + "s, redDuration=" + String(redDuration/1000.0) + "s";

  server.send(200, "text/plain", response);
}

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

  calculateWebsterDurations();

  server.on("/setTrafficData", handleSetTrafficData);
  server.begin();
  Serial.println("HTTP server started");

  switchLED();
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
