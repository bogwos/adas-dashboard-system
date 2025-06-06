#include <SPI.h>
#include <Wire.h>

#define TOUCH_SENSOR_PIN 2
#define POT_PIN A0

const int MPU = 0x68;
float AccX, AccY, AccZ;
int accAngleX;

int touchSensorValue;
int potentiometerValue;

volatile boolean dataReceived = false;
volatile byte command;
byte sendBuffer[8];
byte sendIndex = 0;

ISR(SPI_STC_vect) {
    command = SPDR; // Lire la commande du maître

    if (command == 0xA1) {
        sendBuffer[0] = lowByte(potentiometerValue);  // Non-utilisé
        sendBuffer[1] = highByte(potentiometerValue); // Non-utilisé
        sendBuffer[2] = lowByte(potentiometerValue);
        sendBuffer[3] = highByte(potentiometerValue);
        sendBuffer[4] = lowByte(accAngleX);
        sendBuffer[5] = highByte(accAngleX);
        sendBuffer[6] = lowByte(touchSensorValue);
        sendBuffer[7] = highByte(touchSensorValue);
        sendIndex = 0;
    }

    SPDR = sendBuffer[sendIndex++];

    dataReceived = true;
}

void setup() {
    Serial.begin(9600);

    pinMode(MISO, OUTPUT);
    SPCR |= _BV(SPE);      // Activer SPI en esclave
    SPI.attachInterrupt(); // Interruption SPI activée

    pinMode(TOUCH_SENSOR_PIN, INPUT);

    Wire.begin();
    Wire.beginTransmission(MPU);
    Wire.write(0x6B);
    Wire.write(0x00);
    Wire.endTransmission(true);
}

void loop() {
    touchSensorValue = digitalRead(TOUCH_SENSOR_PIN);
    potentiometerValue = analogRead(POT_PIN);
    potentiometerValue = map(potentiometerValue, 1023, 0, 0, 200);

    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);

    AccX = (Wire.read() << 8 | Wire.read()) / 16384.0;
    AccY = (Wire.read() << 8 | Wire.read()) / 16384.0;
    AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0;
    accAngleX =
        (atan(AccY / sqrt(pow(AccX, 2) + pow(AccZ, 2))) * 180 / PI) - 0.58;

    Serial.print("Touch sensor: " + String(touchSensorValue));
    Serial.print(", Potentiometer: " + String(potentiometerValue));
    Serial.println(", accAngleX: " + String(accAngleX));
}
