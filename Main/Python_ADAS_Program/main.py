import serial
import time

ser = serial.Serial("COM7", 9600)
time.sleep(1)

speed = 100

while True:
    ser.write(bytes([speed]))
    print(f"Sent: speed = {speed}")
    time.sleep(0.5)
