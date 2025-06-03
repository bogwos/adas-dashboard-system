#include <WebServer.h>
#include <WiFi.h>

#include "htmlPage.h"

const char *ssid = "Choaibs-Phone";
const char *pass = "Choaibs-Phone";

WebServer server(80);

void handleRoot();
void handleData();

int readSpeedSensor();
int readTouchSensor();
int readTemperatureSensor();
int readLightSensor();

void setup() {
    Serial.begin(9600);

    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");

    Serial.print("Connected to network: ");
    Serial.print(ssid);
    Serial.print(", IP address: ");
    Serial.println(WiFi.localIP());
    delay(100);

    server.on("/", handleRoot);
    server.on("/data", handleData);
    server.begin();
    Serial.println("Web server started.");
}

void loop() { server.handleClient(); }

void handleRoot() { server.send(200, "text/html", htmlPage); }

void handleData() {
    int speed = readSpeedSensor();
    int touch = readTouchSensor();
    int temperature = readTemperatureSensor();
    int light = readLightSensor();

    String jsonData = "{";
    jsonData += "\"speed\":" + String(speed) + ",";
    jsonData += "\"touch\":" + String(touch) + ",";
    jsonData += "\"temperature\":" + String(temperature) + ",";
    jsonData += "\"light\":" + String(light);
    jsonData += "}";

    server.send(200, "application/json", jsonData);
}

int readSpeedSensor() { return random(0, 201); }
int readTouchSensor() { return random(0, 2); }
int readTemperatureSensor() { return random(-5, 51); }
int readLightSensor() { return random(0, 1001); }
