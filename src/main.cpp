#include "Arduino.h"
#include "SimpleDHT.h"

#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

// Variables
byte temperature = 0;
byte humidity = 0;
int uvIndex = 0;

// Pins
int uvDetectorPin = A0;
int dht22Pin = 2;
SimpleDHT22 dht22(dht22Pin);

// Wifi
AsyncWebServer server(80);
const char* ssid = "ZONG MBB-E8372-95F5";
const char* password = "41473716";

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void setup() 
{
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
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->printf("<br><label>Time: %d</label><br>", (int) millis());
    response->printf("<br><label>UV Index: %d</label><br>", uvIndex);
    response->printf("<br><label>Temperature: %d C</label><br>", (int) temperature);
    response->printf("<br><label>Humidity: %d%% RH</label><br>", (int) humidity);
    request->send(response);
  });
  server.onNotFound(notFound);
  server.begin();
}

void loop() 
{
  Serial.println("=================================");

  // UV Detector
  int sensorValue = analogRead(uvDetectorPin);
  int voltage = (sensorValue * (5.0 / 1023.0)) * 1000; // Voltage in miliVolts
  
  if (voltage > 0 && voltage <= 227) {
    uvIndex = 0;
  } else if (voltage > 227 && voltage <= 318) {
    uvIndex = 1;
  } else if (voltage >318 && voltage <= 408) {
    uvIndex = 2;
  } else if (voltage > 408 && voltage <= 503) {
    uvIndex = 3;
  } else if (voltage > 503 && voltage <= 606) {
    uvIndex = 4;
  } else if (voltage > 606 && voltage <= 696) {
    uvIndex = 5;
  } else if (voltage > 696 && voltage <= 795) {
    uvIndex = 6;
  } else if (voltage > 795 && voltage <= 881) {
    uvIndex = 7;
  } else if (voltage > 881 && voltage <= 976) {
    uvIndex = 8;
  } else if (voltage > 976 && voltage <= 1079) {
    uvIndex = 9;
  } else if (voltage > 1079 && voltage <= 1170) {
    uvIndex = 10;
  } else if (voltage > 1170) {
    uvIndex = 11;
  }
  Serial.print("UV Index: ");
  Serial.println(uvIndex);

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

  delay(2500);
}
