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

#include <TM1637Display.h>

// Variables
uint16_t SLEEP_TIME = 3 * 1000;
uint16_t uvVoltage;
uint16_t highUvVoltage = 0;
uint8_t uvIndex = 0;
uint8_t highUvIndex = 0;
byte temperature = 0;
byte humidity = 0;
int dhtErr = SimpleDHTErrSuccess;

// Pins
uint8_t uvDetectorPin = A0;
uint8_t uvSensorVoltageInput = 3.3;
uint8_t dht22Pin = 2;
SimpleDHT22 dht22(dht22Pin);
uint8_t TM1637_CLK = 2;
uint8_t TM1637_DIO = 3;

// TM1637 Display
TM1637Display display = TM1637Display(TM1637_CLK, TM1637_DIO);
bool displayShowingUvIndex = false;

// Wifi
AsyncWebServer server(80);
const char *ssid = "Jirawat_2.4GHz";
const char *password = "0845916998";
void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

void setup()
{
  Serial.begin(115200);

  // setup the display
  display.clear();
  // Set the brightness:
  display.setBrightness(7);

  // Wifi setup
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("WiFi Failed!\n");
    return;
  }
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
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
    response->printf("<br><label>UV Index: %d highest: %d</label><br>", uvIndex, highUvIndex);
    response->printf("<br><label>UV voltage: %d highest: %d</label><br>", uvVoltage, highUvVoltage);
    response->printf("<br><label>Temperature: %d C</label><br>", (uint8_t) temperature);
    response->printf("<br><label>Humidity: %d%% RH</label><br>", (uint8_t) humidity); // 
    response->printf("<br><label>DHT22 Error: %d</label><br>", dhtErr);
    request->send(response); });

  server.onNotFound(notFound);

  server.begin();
}

uint8_t getUvIndex(uint16_t uvVoltage)
{
  switch (uvVoltage)
  {
  case 0 ... 50:
    return 0;
  case 51 ... 227:
    return 1;
  case 228 ... 318:
    return 2;
  case 319 ... 408:
    return 3;
  case 409 ... 503:
    return 4;
  case 504 ... 606:
    return 5;
  case 607 ... 696:
    return 6;
  case 697 ... 795:
    return 7;
  case 796 ... 881:
    return 8;
  case 882 ... 976:
    return 9;
  case 977 ... 1170:
    return 10;
  default:
    return 11;
  }
}

void loop()
{
  Serial.println("=================================");

  // UV Detector
  uint16_t sensorValue = analogRead(uvDetectorPin);
  uvVoltage = (sensorValue * (uvSensorVoltageInput / 1023.0)) * 1000;
  Serial.printf("UV Voltage: %d\n", uvVoltage);

  uvIndex = getUvIndex(uvVoltage);
  Serial.printf("UV Index: %d\n", uvIndex);

  if (uvVoltage > highUvVoltage)
  {
    highUvVoltage = uvVoltage;
    highUvIndex = uvIndex;
  }

  // Huminity and Temperature DHT22
  dhtErr = dht22.read(&temperature, &humidity, NULL);
  if (dhtErr != SimpleDHTErrSuccess)
  {
    Serial.printf("Read DHT22 failed, err=%d\n", dhtErr);
  }
  else
  {
    Serial.printf("Temperature: %d C\n", (uint8_t)temperature);

    Serial.printf("Humidity: %d%% RH\n", (uint8_t)humidity);
  }

  // update display
  display.clear();
  if (displayShowingUvIndex)
  {
    // show UV index
    display.showNumberDec(uvVoltage, true, 4, 0);

    displayShowingUvIndex = false;

    Serial.println("1111");
  }
  else
  {
    // show temperature and humidity
    display.showNumberDec((uint8_t)temperature, true, 2, 0);
    display.showNumberDec((uint8_t)humidity, true, 2, 2);

    displayShowingUvIndex = true;

    Serial.println("2222");
  }

  delay(SLEEP_TIME);
}
