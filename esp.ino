#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <WebServer.h>

// ============= PIN DEFINITIONS =============
// Change these pins according to your wiring
#define DHTPIN 27        // DHT11 Temperature & Humidity sensor
#define TDS_PIN 32       // TDS (Total Dissolved Solids) sensor analog input
#define PH_PIN 34        // pH sensor analog input  
#define RELAY_PIN 4      // Water pump relay control pin

// ============= DISPLAY SETTINGS =============
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ============= SENSOR INITIALIZATION =============
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ============= WIFI INITIALIZATION =============

const char* ssid = "ESP32-Hydroponics";
const char* password = "12345678";

WebServer server(80);

// ADC and calibration constants
const float ADC_Resolution = 4095.0;
const float Vref = 3.3;
const float NeutralVoltage = 1.627; // Calibrated for pH 7.0
const float TDS_CorrectionFactor = 2.57;     // Multiplier
const float TEMP_Offset = 29.4;              // Additive

// Sensor values intially
float humidity = 0;
float phValue = 0;
float tds = 0;
float temp = 0;
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
  readSensors();  // Read actual sensor values
  updateDisplay();
  delay(3000);
}

void readSensors() {
  // Read raw temperature from DHT11 and apply offset
  float rawTemp = dht.readTemperature();
  if (!isnan(rawTemp)) {
    temp = rawTemp + TEMP_Offset;
  } else {
    Serial.println("Failed to read temperature from DHT sensor!");
    temp = 0; // or keep previous value
  }

  // Read humidity from DHT11
  humidity = dht.readHumidity();
  if (isnan(humidity)) {
    Serial.println("Failed to read humidity from DHT sensor!");
    humidity = 0; // or keep previous value
  }

  // Read TDS sensor
  int tdsRawValue = analogRead(TDS_PIN);
  float tdsVoltage = tdsRawValue * Vref / ADC_Resolution;
  float tdsRaw = (tdsVoltage * 133.42) / (1 + 0.02 * (temp - 25.0));
  tds = tdsRaw * TDS_CorrectionFactor;

  // Read pH sensor
  int phRawValue = analogRead(PH_PIN);
  float phVoltage = (phRawValue / ADC_Resolution) * Vref;
  phValue = 7.0 - ((phVoltage - NeutralVoltage) / 0.0591);

  // Debug output
  Serial.print("Temperature: "); Serial.print(temp); Serial.println(" °C");
  Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %");
  Serial.print("TDS: "); Serial.print(tds); Serial.println(" ppm");
  Serial.print("pH: "); Serial.println(phValue, 2);
  Serial.println("---");
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