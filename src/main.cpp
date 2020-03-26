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
int uvVoltage;
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
    int runHours = allSeconds / 3600;
    int secsRemaining =allSeconds % 3600;
    int runMinutes = secsRemaining / 60;
    int runSeconds = secsRemaining % 60;
    char time[13];
    sprintf(time, "%02d:%02d:%02d", runHours, runMinutes, runSeconds);

    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->printf("<br><label>Runtime: %s</label><br>", time);
    response->printf("<br><label>UV Index: %d</label><br>", uvIndex);
    response->printf("<br><label> UV voltage: %d</label><br>", uvVoltage);
    response->printf("<br><label>Temperature: %d C</label><br>", (int) temperature);
    response->printf("<br><label>Humidity: %d%% RH</label><br>", (int) humidity);
    request->send(response);
  });

  server.onNotFound(notFound);

  server.begin();
}

void loop() {
  Serial.println("=================================");

  // UV Detector
  int sensorValue = analogRead(uvDetectorPin);
  uvVoltage = (sensorValue * (5.0 / 1023.0)) * 1000; //Voltage in miliVolts
  
  switch (uvVoltage) {
    case 0 ... 227:uvIndex = 1;break;
    case 228 ... 318:uvIndex = 2;break;
    case 319 ... 408:uvIndex = 3;break;
    case 409 ... 503:uvIndex = 4;break;
    case 504 ... 606:uvIndex = 5;break;
    case 607 ... 696:uvIndex = 6;break;
    case 697 ... 795:uvIndex = 7;break;
    case 796 ... 881:uvIndex = 8;break;
    case 882 ... 976:uvIndex = 9;break;
    case 977 ... 1170:uvIndex = 10;break;
    default:uvIndex = 11;break; 
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

  delay(3000);
}
