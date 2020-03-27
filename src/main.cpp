#include <stdio.h>
#include <Arduino.h>
#include <SimpleDHT.h>

#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

// Variables
uint16_t SLEEP_TIME = 3 * 1000;
uint16_t uvVoltage;
uint16_t highUvVoltage = 0;
uint8_t uvIndex = 0;
uint8_t highUvIndex = 0;
byte temperature = 0;
byte humidity = 0;

// Pins
uint8_t uvDetectorPin = A0;
uint8_t uvSensorVoltageInput = 3.3;
uint8_t dht22Pin = 2;
SimpleDHT22 dht22(dht22Pin);

// Wifi
AsyncWebServer server(80);
const char* ssid = "ZONG MBB-E8372-95F5";
const char* password = "41473716";
void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(115200);

  // Wifi setup
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.printf("WiFi Failed!\n");
      return;
  }
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Time
    unsigned long allSeconds = millis() / 1000;
    uint8_t runHours = allSeconds / 3600;
    uint8_t secsRemaining = allSeconds % 3600;
    uint8_t runMinutes = secsRemaining / 60;
    uint8_t runSeconds = secsRemaining % 60;
    char time[13];
    sprintf(time, "%02d:%02d:%02d", runHours, runMinutes, runSeconds);

    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->printf("<br><label>Runtime: %s</label><br>", time);
    response->printf("<br><label>UV Index: %d high: %d</label><br>", uvIndex, highUvIndex);
    response->printf("<br><label>UV voltage: %d high: %d</label><br>", uvVoltage, highUvVoltage);
    response->printf("<br><label>Temperature: %d C</label><br>", (uint8_t) temperature);
    response->printf("<br><label>Humidity: %d%% RH</label><br>", (uint8_t) humidity);
    request->send(response);
  });

  server.onNotFound(notFound);

  server.begin();
}

uint8_t getUvIndex(uint16_t uvVoltage) {
  switch (uvVoltage) {
    case 0 ... 50:return 0;
    case 51 ... 227:return 1;
    case 228 ... 318:return 2;
    case 319 ... 408:return 3;
    case 409 ... 503:return 4;
    case 504 ... 606:return 5;
    case 607 ... 696:return 6;
    case 697 ... 795:return 7;
    case 796 ... 881:return 8;
    case 882 ... 976:return 9;
    case 977 ... 1170:return 10;
    default:return 11;
  }
}

void loop() {
  Serial.println("=================================");

  // UV Detector
  uint16_t sensorValue = analogRead(uvDetectorPin);
  uvVoltage = (sensorValue * (uvSensorVoltageInput / 1023.0)) * 1000;
  uvIndex = getUvIndex(uvVoltage);
  
  Serial.print("UV Index: ");
  Serial.println(uvIndex);

  if (uvVoltage > highUvVoltage) {
    highUvVoltage = uvVoltage;
    highUvIndex = uvIndex;
  }

  // Huminity and Temperature DHT22
  int err = SimpleDHTErrSuccess;
  if ((err = dht22.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT22 failed, err=");
    Serial.println(err);
  } else {
    Serial.print("Temperature: ");
    Serial.print((int) temperature); Serial.println(" C");

    Serial.print("Humidity: ");
    Serial.print((int) humidity); Serial.println("% RH");
  }

  delay(SLEEP_TIME);
}
