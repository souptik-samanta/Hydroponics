// with dummy values

#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <WebServer.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define DHTPIN 27
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define RELAY_PIN 4

const char* ssid = "ESP32-Hydroponics";
const char* password = "12345678";

WebServer server(80);

// Dummy Values
float humidity = 6;
float phValue = 6;
float tds = 6;
float temp = 6;
bool pumpStatus = true;

void setup() {
  Serial.begin(115200);
  Serial.println("Booting...");

  dht.begin();
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // ON
  Serial.println("Pump set to ON");

  // OLED Init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Starting...");
    display.display();
  }

  // WiFi AP Mode
  WiFi.softAP(ssid, password);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  // API: GET /get
  server.on("/get", HTTP_GET, []() {
    String response = "{";
    response += "\"ph\":" + String(phValue, 2) + ",";
    response += "\"tds\":" + String(tds, 1) + ",";
    response += "\"temp\":" + String(temp, 1) + ",";
    response += "\"humidity\":" + String(humidity, 1) + ",";
    response += "\"pump\":" + String(pumpStatus ? "true" : "false");
    response += "}";
    server.send(200, "application/json", response);
  });

  // API: POST /pump
  server.on("/pump", HTTP_POST, []() {
    if (server.hasArg("status")) {
      String val = server.arg("status");
      if (val == "on") {
        digitalWrite(RELAY_PIN, HIGH);
        pumpStatus = true;
      } else {
        digitalWrite(RELAY_PIN, LOW);
        pumpStatus = false;
      }
      server.send(200, "text/plain", "Pump: " + val);
    } else {
      server.send(400, "text/plain", "Missing 'status'");
    }
  });

  server.begin();
  Serial.println("Server started");
}

void loop() {
  server.handleClient();
  updateDisplay();
  delay(3000);
}

void updateDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("pH: "); display.println(phValue, 2);
  display.print("TDS: "); display.println(tds);
  display.print("Temp: "); display.println(temp);
  display.print("Humid: "); display.println(humidity);
  display.print("Pump: "); display.println(pumpStatus ? "ON" : "OFF");
  display.display();
}
