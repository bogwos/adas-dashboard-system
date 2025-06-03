#include <Wire.h>

#define I2C_SLAVE_ADDR 0x42

String messageToSend = "Hi, STM32!";

void receiveEvent(int);
void requestEvent();

void setup() {
    Serial.begin(115200);
    Wire.begin(I2C_SLAVE_ADDR);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);
    Serial.println("ESP32 I2C Slave Ready!");
}

void loop() { delay(100); }

void receiveEvent(int len) {
    while (Wire.available()) {
        char c = Wire.read();
        Serial.print(c);
    }
    Serial.println();
}

void requestEvent() { Wire.write(messageToSend.c_str()); }
