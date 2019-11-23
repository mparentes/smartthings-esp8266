#include <Arduino.h>
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <WiFiClient.h>

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

#include <ESP8266WebServer.h>
#include <ESP8266SSDP.h>


// Define All Devices
#define RELAY1  D0  //Bultin LED
#define RELAY2  D4  //Bultin ESP12 LED

#define SMARTTHINGS_IP    "192.168.10.20"
#define SMARTTHINGS_PORT  39500

// Variables 
unsigned long   currentMillis = 0;
unsigned long   previousStatusMillis = 0;


const char * projectName = "NodeMCU - Custom Device";

void handleRoot();
void handleNotFound();
void handleControl();
void handleSSDP();
void sendStatus();
void runEveryMinute();
String uptime();

ESP8266WebServer server(80);

void setup() {

  Serial.begin(115200);

  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  
  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);

  WiFiManager wifiManager;

  wifiManager.autoConnect("AutoConnectAP");
  Serial.println("connected...yeey :)");

  server.on("/", handleRoot);
  server.on("/description.xml", handleSSDP);
  server.on("/control", handleControl);

  server.onNotFound(handleNotFound);

  server.begin();                           // Actually start the server
  Serial.println("HTTP server started");

  //SSDP
  Serial.printf("Starting SSDP...\n");
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.setName(projectName);
  SSDP.setSerialNumber(ESP.getChipId());
  SSDP.setURL("index.html");
  SSDP.setModelName(projectName);
  SSDP.setModelNumber("node-mcu-001");
  SSDP.setModelURL("https://www.eztechpc.com");
  SSDP.setManufacturer("EztechPC ESP8266 Device");
  SSDP.setManufacturerURL("https://www.eztechpc.com");
  SSDP.begin();
  Serial.println("Done");

}

void loop() {
  server.handleClient();  
  runEveryMinute();
}

void runEveryMinute() {
  currentMillis = millis();
  if (millis() - previousStatusMillis >= 60000) {
    previousStatusMillis += 60000;

    int currentState1 = digitalRead(RELAY1);
    int currentState2 = digitalRead(RELAY2);

    String message = "";  
    message += "{\"type\":\"relay\", \"number\":\"1\", \"power\":\"" + String(currentState1 == 0? "on" : "off") + "\"}";
    message += "{\"type\":\"relay\", \"number\":\"2\", \"power\":\"" + String(currentState2 == 0? "on" : "off") + "\"}";
    message += "{\"type\":\"uptime\", \"value\":\"" + uptime() + "\"}";
    message += "{\"type\":\"ip\", \"value\":\"" + WiFi.localIP().toString() + "\"}";
    message += "]";
  //              client.print(String("POST ") + url + " HTTP/1.1\r\n" +
  //              "Host: " + host + ":" + Settings.haPort + "\r\n" + authHeader +
  //              "Content-Type: application/json;charset=utf-8\r\n" +
  //              "Content-Length: " + message.length() + "\r\n" +
  //              "Server: " + projectName + "\r\n" +
  //              "Connection: close\r\n\r\n" +
  //              message + "\r\n");

  Serial.println("Sending Status to Controller");
  Serial.println(message);
  
  }
  
}

void sendStatus(int sensor) {
    String message;
    int currentState = digitalRead(sensor);
    message = "{\"type\":\"relay\", \"number\":\"" + String(sensor) + "\", \"power\":\"" + String(currentState == 0? "on" : "off") + "\"}";  
    server.send(200, "application/json", message);
}

void handleControl() {
  String value;

    if (server.hasArg("relay1")) { //Switch 1
      value = server.arg("relay1");
      byte state = (value == "on") ? LOW : HIGH;
      digitalWrite(RELAY1, state);
      sendStatus(RELAY1);
    }

    if (server.hasArg("relay2")) { //Switch 2
      value = server.arg("relay2");
      byte state = (value == "on") ? LOW : HIGH;
      digitalWrite(RELAY2, state);
      sendStatus(RELAY2);
    }    

}

void handleRoot() {
  server.send(200, "application/json", "{\"message\":\"NodeMCU\"}");
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); 
}

void handleSSDP() {
  SSDP.schema(server.client());
}



String uptime()
{
  currentMillis = millis();
  long days = 0;
  long hours = 0;
  long mins = 0;
  long secs = 0;
  secs = currentMillis / 1000; //convect milliseconds to seconds
  mins = secs / 60; //convert seconds to minutes
  hours = mins / 60; //convert minutes to hours
  days = hours / 24; //convert hours to days
  secs = secs - (mins * 60); //subtract the coverted seconds to minutes in order to display 59 secs max
  mins = mins - (hours * 60); //subtract the coverted minutes to hours in order to display 59 minutes max
  hours = hours - (days * 24); //subtract the coverted hours to days in order to display 23 hours max

  if (days > 0) // days will displayed only if value is greater than zero
  {
    return String(days) + " days and " + String(hours) + ":" + String(mins) + ":" + String(secs);
  } else {
    return String(hours) + ":" + String(mins) + ":" + String(secs);
  }
}

