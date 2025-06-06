#include <SPI.h>

// MISO: Pin 12
// MOSI: Pin 11
// SCK:  Pin 13
// SS:   Pin 10

#define CMD_GET_SENSORS 0x01
#define CMD_RESET 0x02
#define CMD_STATUS 0x03

struct SensorResponse {
    int value1;
    int value2;
    int value3;
};

SensorResponse sensorData;
volatile bool commandReceived = false;
volatile uint8_t receivedCommand = 0;

void setup() {
    Serial.begin(9600);
    Serial.println("Arduino Nano SPI Slave - Démarrage...");

    pinMode(SS, INPUT_PULLUP);

    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV16);

    SPCR |= _BV(SPIE);

    updateSensorValues();

    Serial.println("Arduino prêt à recevoir des commandes...");
}

void loop() {
    updateSensorValues();

    if (commandReceived) {
        handleCommand(receivedCommand);
        commandReceived = false;
    }

    delay(10);
}

// Interruption SPI - appelée quand des données sont reçues
ISR(SPI_STC_vect) {
    uint8_t data = SPDR;

    if (!commandReceived) {
        receivedCommand = data;
        commandReceived = true;
        SPDR = ((uint8_t*)&sensorData)[0];
    } else {
        static int byteIndex = 1;
        if (byteIndex < sizeof(SensorResponse)) {
            SPDR = ((uint8_t*)&sensorData)[byteIndex];
            byteIndex++;
        } else {
            byteIndex = 1;
        }
    }
}

void handleCommand(uint8_t command) {
    switch (command) {
        case CMD_GET_SENSORS:
            Serial.println("Commande GET_SENSORS reçue");
            Serial.print("Envoi: ");
            Serial.print(sensorData.value1);
            Serial.print(", ");
            Serial.print(sensorData.value2);
            Serial.print(", ");
            Serial.println(sensorData.value3);
            break;
            
        case CMD_RESET:
            Serial.println("Commande RESET reçue");
            resetSensors();
            break;
            
        case CMD_STATUS:
            Serial.println("Commande STATUS reçue");
            sensorData.value1 = 1;  // Status OK
            sensorData.value2 = 0;
            sensorData.value3 = 0;
            break;
            
        default:
            Serial.println("Commande inconnue: " + String(command));
            break;
    }
}

void updateSensorValues() {
    sensorData.value1 = analogRead(A0);
    sensorData.value2 = analogRead(A1);
    sensorData.value3 = random(100, 1000);
}

void resetSensors() {
    sensorData.value1 = 0;
    sensorData.value2 = 0;
    sensorData.value3 = 0;
    Serial.println("Capteurs remis à zéro");
}
