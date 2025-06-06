#include <Wire.h>

const int MPU = 0x68;
float AccX, AccY, AccZ;
float GyroX;
float accAngleX;
float roll;

void setup() {
    Serial.begin(19200);

    Wire.begin();
    Wire.beginTransmission(MPU);
    Wire.write(0x6B);
    Wire.write(0x00);
    Wire.endTransmission(true);
}
void loop() {
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);

    AccX = (Wire.read() << 8 | Wire.read()) / 16384.0; // X-axis value
    AccY = (Wire.read() << 8 | Wire.read()) / 16384.0; // Y-axis value
    AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0; // Z-axis value
    accAngleX =
        (atan(AccY / sqrt(pow(AccX, 2) + pow(AccZ, 2))) * 180 / PI) - 0.58;

    Wire.beginTransmission(MPU);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);

    GyroX = (Wire.read() << 8 | Wire.read()) / 131.0;
    GyroX = GyroX + 0.56;
    gyroAngleX = gyroAngleX + GyroX * elapsedTime;
    roll = 0.96 * gyroAngleX + 0.04 * accAngleX;
    Serial.println(roll);
}
