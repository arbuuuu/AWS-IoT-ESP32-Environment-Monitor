#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "DHT.h"
#include "WiFi.h"

// ============================================================
// ===================  USER CONFIGURATION  ===================
// ============================================================

// ⚠️ GITHUB SECURITY NOTE:
// Real credentials have been removed for security.
// Replace "YOUR_..." with actual values before flashing to ESP32.

// 1. WIFI SETTINGS
const char WIFI_SSID[] = "YOUR_WIFI_NAME";       // <--- Sirf yahan change karna jab chalao
const char WIFI_PASSWORD[] = "YOUR_WIFI_PASSWORD"; // <--- Yahan Password

// 2. AWS IOT SETTINGS
const char AWS_IOT_ENDPOINT[] = "YOUR_AWS_ENDPOINT_HERE"; // <--- AWS Endpoint
const char AWS_IOT_TOPIC[]    = "esp32/status";

// 3. AMAZON ROOT CA 1
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
PASTE_YOUR_ROOT_CA_HERE
-----END CERTIFICATE-----
)EOF";

// 4. DEVICE CERTIFICATE
static const char AWS_CERT_CRT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
PASTE_YOUR_DEVICE_CERTIFICATE_HERE
-----END CERTIFICATE-----
)EOF";

// 5. PRIVATE KEY
static const char AWS_CERT_PRIVATE[] PROGMEM = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
PASTE_YOUR_PRIVATE_KEY_HERE
-----END RSA PRIVATE KEY-----
)EOF";

// ============================================================
// ===================    HARDWARE SETUP    ===================
// ============================================================

#define DHTPIN 5      
#define DHTTYPE DHT11 

const int ledPins[4] = {25, 26, 27, 4};     
const int buttonPins[4] = {12, 13, 14, 15}; 

// --- GLOBAL VARIABLES ---
DHT dht(DHTPIN, DHTTYPE); 
WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);

unsigned long ledOnTimes[4] = {0, 0, 0, 0}; 
bool lastButtonStates[4] = {HIGH, HIGH, HIGH, HIGH}; 
const unsigned long glowDuration = 5000; 

// Timer variables for DHT
unsigned long previousDHTMillis = 0;
const long dhtInterval = 2000; 

// ============================================================
// ===================    AWS FUNCTIONS     ===================
// ============================================================

void connectAWS() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  // Certs Config
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  Serial.println("Connecting to AWS IoT");
  while (!client.connect("ESP32_Sensor_Client")) {
    Serial.print(".");
    delay(1000);
  }

  if (!client.connected()) {
    Serial.println("AWS IoT Timeout!");
    return;
  }
  Serial.println("\nAWS IoT Connected!");
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Setup Hardware
  for (int i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }
  
  for (int i = 0; i < 4; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  connectAWS();
}

void loop() {
  client.loop();
  
  if (!client.connected()) {
    Serial.println("Reconnecting...");
    connectAWS();
  }

  unsigned long currentMillis = millis(); 

  // Buttons & LEDs Logic
  for (int i = 0; i < 4; i++) {
    bool currentButtonState = digitalRead(buttonPins[i]);
    if (currentButtonState == LOW && lastButtonStates[i] == HIGH) {
      Serial.print("Button Pressed: ");
      Serial.println(i);
      digitalWrite(ledPins[i], HIGH);
      ledOnTimes[i] = currentMillis; 
    }
    lastButtonStates[i] = currentButtonState;
  }
  
  // LED Timer Logic
  for (int i = 0; i < 4; i++) {
    if (ledOnTimes[i] != 0 && (currentMillis - ledOnTimes[i] >= glowDuration)) {
      digitalWrite(ledPins[i], LOW);
      ledOnTimes[i] = 0; 
      Serial.print("LED OFF: ");
      Serial.println(i);
    }
  }

  // Sensor Logic
  if (currentMillis - previousDHTMillis >= dhtInterval) {
    previousDHTMillis = currentMillis;
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (!isnan(h) && !isnan(t)) {
      StaticJsonDocument<200> doc;
      doc["humidity"] = h;
      doc["temperature"] = t;
      doc["leds_active"] = (ledOnTimes[0]>0) + (ledOnTimes[1]>0) + (ledOnTimes[2]>0) + (ledOnTimes[3]>0);

      char jsonBuffer[512];
      serializeJson(doc, jsonBuffer);

      if(client.publish(AWS_IOT_TOPIC, jsonBuffer)) {
         Serial.println("Data sent to AWS!"); 
      } else {
         Serial.println("Publish failed!");
      }
    }
  }
}