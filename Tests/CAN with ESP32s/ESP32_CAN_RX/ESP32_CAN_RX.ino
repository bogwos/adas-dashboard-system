#include <CAN.h>

void setup() {
    Serial.begin(115200);
    while (!Serial) {
    }

    if (!CAN.begin(500E3)) {
        Serial.println("CAN init failed!");
        while (1) {
        }
    }

    Serial.println("ESP32 #2 ready: CAN→Serial");
}

void loop() {
    if (CAN.available()) {
        int packetSize = CAN.parsePacket();
        if (packetSize && CAN.packetId() == 0x01) {
            int speed = CAN.read();
            Serial.print("Received speed: ");
            Serial.println(speed);
        }
    }
}
