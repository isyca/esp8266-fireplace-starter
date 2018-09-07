/*
  Fireplace Starter (ESP8266)

  ESP8266 module that sends a remote control signal to start a fireplace

  Created 31 August 2018
  By Ivan Sy
  Modified 6 September 2018
  By Ivan Sy

  https://github.com/isyca/esp8266-fireplace-starter
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <RCSwitch.h>

RCSwitch txSwitch = RCSwitch();
ESP8266WebServer webServer(80);

const char* ssid = "SSID";
const char* password = "password";
const char* hostnameStr = "fireplace";

const int transmitPin = 0;
const int externalLED = 2;

const int pulseLength = 360;

int lightStatusInt = 0;
int heatStatusInt = 0;

void turnFirePlaceON ()
{
  txSwitch.setRepeatTransmit(2);
  txSwitch.send("000001110011010111000101");
  txSwitch.setRepeatTransmit(1);
  txSwitch.send("000001110011010111");
}

void turnFirePlaceOFF ()
{
  txSwitch.setRepeatTransmit(3);
  txSwitch.send("000001110011010100110101");
  txSwitch.setRepeatTransmit(1);
  txSwitch.send("00000");
}

void handleRoot() {
  webServer.send(200, "application/json", "{\"result\": 0, \"msglevel\": \"ERROR\", \"msg\": \"Access ROOT nothing here\"}");
}

void handleNotFound() {
  String message = "File Not Found = ";
  message += " URI: [";
  message += webServer.uri();
  message += "] Method: [";
  message += (webServer.method() == HTTP_GET) ? "GET" : "POST";
  message += "] Arguments: [";
  message += webServer.args();
  message += "]";
  for (uint8_t i = 0; i < webServer.args(); i++) {
    message += "(" + webServer.argName(i) + ": " + webServer.arg(i) + ")";
  }
  webServer.send(404, "application/json", "{\"result\": 0, \"msglevel\": \"ERROR\", \"msg\": \"" + message + "\"}");
}

void blinkExternalLED(int count, int remainHigh) //count = 5 means approx 1 second delay
{
  while (count > 0)
  {
    digitalWrite(externalLED, LOW);
    delay(100);
    digitalWrite(externalLED, HIGH);
    count = count -1;    
  }

  if (remainHigh = 1)
  {
    digitalWrite(externalLED, HIGH);
  } else {
    digitalWrite(externalLED, LOW);
  }
}

void setup(void) {
  pinMode(externalLED, OUTPUT);

  WiFi.hostname(hostnameStr);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  txSwitch.enableTransmit(transmitPin);
  txSwitch.setPulseLength(pulseLength);

  while (WiFi.status() != WL_CONNECTED) {    
    blinkExternalLED(25,0);   // ~5 seconds delay
  }

  webServer.on("/", handleRoot);

  webServer.on("/on", []() {
    blinkExternalLED(2,1);
    turnFirePlaceON();
    lightStatusInt = 1;
    webServer.send(200, "application/json", "{\"result\": 1, \"msglevel\": \"INFO\", \"msg\": \"Fireplace on\"}");
  });

  webServer.on("/off", []() {
    blinkExternalLED(2,1);
    turnFirePlaceOFF();
    lightStatusInt = 0;
    heatStatusInt = 0;
    webServer.send(200, "application/json", "{\"result\": 1, \"msglevel\": \"INFO\", \"msg\": \"Fireplace off\"}");
  });

  webServer.on("/heaton", []() {       // off, turn on, then turn on again
    blinkExternalLED(2,1);
    if (lightStatusInt = 1)
    {
      turnFirePlaceON();
      heatStatusInt = 1;
      webServer.send(200, "application/json", "{\"result\": 1, \"msglevel\": \"INFO\", \"msg\": \"Fireplace Heat on\"}");
    } else {
      if (heatStatusInt = 1)
      {
        turnFirePlaceOFF();
        delay(500);
      }
      turnFirePlaceON();
      delay(300);
      turnFirePlaceON();
      heatStatusInt = 1;
      lightStatusInt = 1;
      webServer.send(200, "application/json", "{\"result\": 1, \"msglevel\": \"INFO\", \"msg\": \"Fireplace Heat on\"}");
    }
  });

  webServer.on("/heatoff", []() {       // completely turn it off (same as /off)
    blinkExternalLED(2,1);
    turnFirePlaceOFF();
    lightStatusInt = 0;
    heatStatusInt = 0;
    webServer.send(200, "application/json", "{\"result\": 1, \"msglevel\": \"INFO\", \"msg\": \"Fireplace Heat off\"}");
  });

  webServer.on("/status", []() {   
    String statusStr = String("{\"light\": " + String(lightStatusInt) + ", \"heat\":" + String(heatStatusInt) + "}");
    webServer.send(200, "application/json", statusStr);
  });

  webServer.on("/homebridge-lightstatus", []() {   
    webServer.send(200, "text/plain", String(lightStatusInt));
  });

  webServer.on("/homebridge-heatstatus", []() {   
    webServer.send(200, "text/plain", String(heatStatusInt));
  });

  webServer.onNotFound(handleNotFound);

  webServer.begin();

  MDNS.begin(hostnameStr);
}

void loop(void) {
  digitalWrite(externalLED, HIGH);
  webServer.handleClient();
}
