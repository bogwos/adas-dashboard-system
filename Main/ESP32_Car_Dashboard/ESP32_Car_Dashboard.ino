#include <CAN.h>
#include <WebServer.h>
#include <WiFi.h>

#include "htmlPage.h"

const char *ssid = "Choaibs-Phone";
const char *pass = "Choaibs-Phone";

int touchSensorValue;
int speed;
int accAngleX;
int maxSpeed = 60;
int recentSpeed = 0;

WebServer server(80);

void handleRoot();
void handleData();

void setup() {
    Serial.begin(115200);
    while (!Serial) {
    }

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
    Serial.println("Web server started!");

    if (!CAN.begin(500E3)) {
        Serial.println("CAN init failed!");
        while (1) {
        }
    }
    Serial.println("CAN with ESP32 started!");
}

void loop() {
    server.handleClient();

    // if (Serial.available()) {
    if (1) {
        // maxSpeed = Serial.read();
        maxSpeed = 200;
        Serial.println("Received from PC: " + String(maxSpeed));

        CAN.beginPacket(0x01); // ID 0x01 for speed messages
        CAN.write(maxSpeed);
        CAN.endPacket();
        Serial.println("Sent over CAN!");
    }

    int packetSize = CAN.parsePacket();
    if (packetSize && CAN.packetId() == 0x02) {
        byte receivedData[6];
        int index = 0;

        while (CAN.available() && index < 6) {
            receivedData[index++] = CAN.read();
        }

        speed = receivedData[0] | (receivedData[1] << 8);
        accAngleX = receivedData[2] | (receivedData[3] << 8);
        touchSensorValue = receivedData[4] | (receivedData[5] << 8);

        if (accAngleX >= 60000) {
            accAngleX = accAngleX - 65535 - 1;
        }

        Serial.print("Data: touchSensorValue = " + String(touchSensorValue));
        Serial.print(", speed = " + String(speed));
        Serial.println(", accAngleX = " + String(accAngleX));
    }
}

void handleRoot() { server.send(200, "text/html", htmlPage); }

void handleData() {
    String jsonData = "{";
    jsonData += "\"speed\":" + String(speed) + ",";
    jsonData += "\"maxSpeed\":" + String(maxSpeed) + ",";
    jsonData += "\"accAngleX\":" + String(accAngleX) + ",";
    jsonData += "\"touchSensorValue\":" + String(touchSensorValue);
    jsonData += "}";

    server.send(200, "application/json", jsonData);
}
