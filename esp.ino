#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Pin definitions
#define DHTPIN 27
#define DHTTYPE DHT22
#define PUMP_PIN 26
#define PH_PIN 34
#define TDS_PIN 32



float TEMPA = 31.0;

const float VREF = 3.3;
const float ADC_RESOLUTION = 4095.0;

//  FINAL calibration factor to get 150 ppm
const float TDS_CALIB_FACTOR = 2.857;


// Display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Sensor and state variables
DHT dht(DHTPIN, DHTTYPE);
float temp = 0, humidity = 0, phValue = 0, tds = 0;
bool pumpState = true;

// Wi-Fi AP mode credentials
const char* ssid = "ESP32-Hydroponics";
const char* password = "12345678";

WebServer server(80);

// Read all sensor values
void readSensors() {
    float temp = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temp) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.printf("Temperature: %.1f C, Humidity: %.1f %%\n", temp, humidity);
  }  

  int phADC = analogRead(PH_PIN);
  float phVoltage = (phADC / 4095.0) * 3.3;
  phValue =9.73 * phVoltage - 11;


  int tdsADC = analogRead(TDS_PIN);
  float tdsVoltage = tdsADC * VREF / ADC_RESOLUTION;
  float rawTDS = (tdsVoltage * 133.42) / (1 + 0.02 * (TEMPA - 25.0));
  float tds = rawTDS * TDS_CALIB_FACTOR;


  Serial.printf("Temp: %.2f C, Humidity: %.2f %%, pH: %.2f, TDS: %.2f ppm\n", temp, humidity, phValue, tds);
}

// Update OLED display
void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.printf("Temp: %.1f C\n", temp);
  display.printf("Hum:  %.1f %%\n", humidity);
  display.printf("pH:   %.2f\n", phValue);
  display.printf("TDS:  %.1f ppm\n", tds);
  display.printf("Pump: %s\n", pumpState ? "ON" : "OFF");

  display.display();
}

// HTTP GET: return JSON sensor data
void handleGet() {
  String json = "{";
  json += "\"ph\":" + String(phValue, 2) + ",";
  json += "\"tds\":" + String(tds, 1) + ",";
  json += "\"temp\":" + String(temp, 1) + ",";
  json += "\"humidity\":" + String(humidity, 1) + ",";
  json += "\"pump\":" + String(pumpState ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

// HTTP POST: control pump
void handlePumpControl() {
  if (server.hasArg("status")) {
    String status = server.arg("status");
    if (status == "on") {
      digitalWrite(PUMP_PIN, HIGH);
      pumpState = true;
      server.send(200, "text/plain", "Pump turned ON");
    } else if (status == "off") {
      digitalWrite(PUMP_PIN, LOW);
      pumpState = false;
      server.send(200, "text/plain", "Pump turned OFF");
    } else {
      server.send(400, "text/plain", "Invalid status");
    }
  } else {
    server.send(400, "text/plain", "Missing 'status' parameter");
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, HIGH);  // Default ON
  pumpState = true;

  // Start OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED init failed"));
    while (true);
  }
  display.clearDisplay();
  display.display();

  // Set up WiFi Access Point
  WiFi.softAP(ssid, password);
  Serial.println("ESP32 in AP mode");
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  // Define server routes
  server.on("/get", HTTP_GET, handleGet);
  server.on("/pump", HTTP_POST, handlePumpControl);
  server.begin();
  Serial.println("Web server started");
}

void loop() {
  server.handleClient();
  readSensors();
  updateDisplay();
  delay(3000);
}