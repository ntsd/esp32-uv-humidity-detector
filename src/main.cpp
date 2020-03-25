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
int uvValue = 0;

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
    response->printf("UV Value: %d", uvValue);
    response->printf("Temperature: %d C", (int) temperature);
    response->printf("Humidity: %d% RH", (int) humidity);
    request->send(response);
  });
  server.onNotFound(notFound);
  server.begin();
}

void loop() 
{
  Serial.println("=================================");

  // UV Detector
  uvValue = analogRead(uvDetectorPin);
  Serial.print("UV Value: ");
  Serial.println(uvValue);

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
