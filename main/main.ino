#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();
ESP8266WebServer server(80);

const char* ssid = "SSID";
const char* password = "password";

const int transmitPin = 0;
const int externalLED = 2;

const int pulseLength = 360;

int lightStatusInt = 0;
int heatStatusInt = 0;

void handleRoot() {
  server.send(200, "application/json", "{\"result\": 0, \"msglevel\": \"ERROR\", \"msg\": \"Access ROOT nothing here\"}");
}

void turnON ()
{
    mySwitch.setRepeatTransmit(2);
    mySwitch.send("000001110011010111000101");
    mySwitch.setRepeatTransmit(1);
    mySwitch.send("000001110011010111");
    lightStatusInt = 1;
}

void turnOFF ()
{
    mySwitch.setRepeatTransmit(3);
    mySwitch.send("000001110011010100110101");
    mySwitch.setRepeatTransmit(1);
    mySwitch.send("00000");
    lightStatusInt = 0;
    heatStatusInt = 0;
}

void handleNotFound() {
  String message = "File Not Found == ";
  message += "URI: [";
  message += server.uri();
  message += "] Method: [";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "] Arguments: [";
  message += server.args();
  message += "]";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += "(" + server.argName(i) + ": " + server.arg(i) + ")";
  }
  server.send(404, "application/json", "{\"result\": 0, \"msglevel\": \"ERROR\", \"msg\": \"" + message + "\"}");
}

//count = 5 means approx 1 second delay
void blinkExternalLED(int count, int remainHigh) 
{
  while (count > 0)
  {
    digitalWrite(externalLED, HIGH);
    delay(100);
    digitalWrite(externalLED, LOW);
    delay(100);
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
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  mySwitch.enableTransmit(transmitPin);
  mySwitch.setRepeatTransmit(2);
  mySwitch.setPulseLength(pulseLength);

  while (WiFi.status() != WL_CONNECTED) {    
    blinkExternalLED(25,0);   // ~5 seconds delay
  }
  server.on("/", handleRoot);

  server.on("/on", []() {
    blinkExternalLED(5,1);
    turnON();
    server.send(200, "application/json", "{\"result\": 1, \"msglevel\": \"INFO\", \"msg\": \"Fireplace on\"}");
  });

  server.on("/off", []() {
    blinkExternalLED(5,1);
    turnOFF();
    server.send(200, "application/json", "{\"result\": 1, \"msglevel\": \"INFO\", \"msg\": \"Fireplace off\"}");
  });

  server.on("/heaton", []() {
    // off, turn on, then turn on again
    blinkExternalLED(5,1);
    turnOFF();
    turnON();
    turnON();  // 2nd turn on will actually do the heat
    heatStatusInt = 1;
    server.send(200, "application/json", "{\"result\": 1, \"msglevel\": \"INFO\", \"msg\": \"Fireplace Heat on\"}");
  });

 server.on("/heatoff", []() {
    blinkExternalLED(5,1);
    turnOFF();
    server.send(200, "application/json", "{\"result\": 1, \"msglevel\": \"INFO\", \"msg\": \"Fireplace Heat off\"}");
  });

  server.on("/status", []() {   
    String statusStr = String("{\"light\": " + String(lightStatusInt) + ", \"heat\":" + String(heatStatusInt) + "}");
    server.send(200, "application/json", statusStr);
  });
  
  server.on("/homebridge-lightstatus", []() {   
    server.send(200, "text/plain", String(lightStatusInt));
  });

  server.on("/homebridge-heatstatus", []() {   
    server.send(200, "text/plain", String(heatStatusInt));
  });


 server.onNotFound(handleNotFound);
 server.begin();
 MDNS.begin("IoTfireplace");

}

void loop(void) {
  digitalWrite(externalLED, HIGH);
  server.handleClient();
}
