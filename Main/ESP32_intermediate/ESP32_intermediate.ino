#include <CAN.h>
#include <SPI.h>
#include <Wire.h>

#define I2C_SLAVE_ADDR 0x42
#define SS_PIN 17

int speed;
int potentiometerValue;
int accAngleX;
int touchSensorValue;

void requestEvent();

void setup() {
    Serial.begin(115200);

    SPI.begin(18, 19, 23, SS_PIN); // SCK, MISO, MOSI, SS
    pinMode(SS_PIN, OUTPUT);
    digitalWrite(SS_PIN, HIGH);

    Wire.begin(I2C_SLAVE_ADDR);
    Wire.onRequest(requestEvent);
    Serial.println("ESP32 I2C slave ready!");

    if (!CAN.begin(500E3)) {
        Serial.println("CAN init failed!");
        while (1) {
        }
    }

    Serial.println("CAN with ESP32 started!");
}

void loop() {
    int packetSize = CAN.parsePacket();
    if (packetSize && CAN.packetId() == 0x01) {
        if (CAN.available()) {
            speed = CAN.read();
            Serial.print("Received speed: ");
            Serial.println(speed);
        }
    }

    byte command = 0xA1;
    byte buffer[8];

    digitalWrite(SS_PIN, LOW);
    delayMicroseconds(10);

    SPI.transfer(command);
    for (int i = 0; i < 8; i++) {
        buffer[i] = SPI.transfer(0x00);
    }

    digitalWrite(SS_PIN, HIGH);

    int NOT_USED = buffer[0] | (buffer[1] << 8);
    potentiometerValue = buffer[2] | (buffer[3] << 8);
    accAngleX = buffer[4] | (buffer[5] << 8);
    touchSensorValue = buffer[6] | (buffer[7] << 8);

    if (accAngleX >= 60000) {
        accAngleX = accAngleX - 65535 - 1;
    }

    Serial.print("Data: touchSensorValue = " + String(touchSensorValue));
    Serial.print(", potentiometerValue = " + String(potentiometerValue));
    Serial.println(", accAngleX = " + String(accAngleX));

    CAN.beginPacket(0x02);
    CAN.write(buffer[2]);
    CAN.write(buffer[3]);
    CAN.write(buffer[4]);
    CAN.write(buffer[5]);
    CAN.write(buffer[6]);
    CAN.write(buffer[7]);
    CAN.endPacket();

    delay(100);
}

void requestEvent() {
    Wire.write(potentiometerValue < speed ? potentiometerValue : speed);
}
