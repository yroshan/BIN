#include <SPI.h>

// ICM-20948 Registers
#define WHO_AM_I 0x00
#define PWR_MGMT_1 0x06
#define ACCEL_CONFIG2 0x14
#define ACCEL_XOUT_H 0x2D
#define ACCEL_XOUT_L 0x2E
#define ACCEL_YOUT_H 0x2F
#define ACCEL_YOUT_L 0x30
#define ACCEL_ZOUT_H 0x31
#define ACCEL_ZOUT_L 0x32

// SPI Settings
const int CS_PIN = 10;
const float ACCEL_SCALE = 16384.0;  // ±2g → 16384 LSB/g

void setup() {
  Serial.begin(115200);

  // Initialize SPI
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);  // CS high to start
  SPI.begin();
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));

  // Wake up ICM-20948
  writeRegister(PWR_MGMT_1, 0x01);  // Set clock source

  // Set LPF to 50.4 Hz (DLPF_CFG = 2)
  writeRegister(ACCEL_CONFIG2, 0x02);

  // Check WHO_AM_I
  uint8_t whoAmI = readRegister(WHO_AM_I);
  Serial.print("WHO_AM_I: 0x");
  Serial.println(whoAmI, HEX);

  if (whoAmI != 0xEA) {
    Serial.println("ICM-20948 not detected. Check wiring!");
    while (1);
  }
  Serial.println("ICM-20948 detected!");
}

void loop() {
  int16_t accelX_raw = (readRegister(ACCEL_XOUT_H) << 8) | readRegister(ACCEL_XOUT_L);
  int16_t accelY_raw = (readRegister(ACCEL_YOUT_H) << 8) | readRegister(ACCEL_YOUT_L);
  int16_t accelZ_raw = (readRegister(ACCEL_ZOUT_H) << 8) | readRegister(ACCEL_ZOUT_L);

  // Convert raw values to g
  float accelX = (float)accelX_raw / ACCEL_SCALE;
  float accelY = (float)accelY_raw / ACCEL_SCALE;
  float accelZ = (float)accelZ_raw / ACCEL_SCALE;

  Serial.print("Accel X: ");
  Serial.print(accelX);
  Serial.print("g, Y: ");
  Serial.print(accelY);
  Serial.print("g, Z: ");
  Serial.print(accelZ);
  Serial.println("g");

 // delay(500);
}

uint8_t readRegister(uint8_t reg) {
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(reg | 0x80);  // Read operation: MSB = 1
  uint8_t data = SPI.transfer(0x00);
  digitalWrite(CS_PIN, HIGH);
  return data;
}

void writeRegister(uint8_t reg, uint8_t data) {
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(reg & 0x7F);  // Write operation: MSB = 0
  SPI.transfer(data);
  digitalWrite(CS_PIN, HIGH);
}


/*
DLPF Bandwidth Options:
DLPF_CFG  Bandwidth (Hz)  Delay (ms)  ODR (kHz)
0 246.0 0.59  1.125
1 111.4 0.88  1.125
2 50.4  1.94  1.125
3 23.9  3.88  1.125
4 11.5  7.80  1.125
5 5.7 15.70 1.125
6 473.0 0.27  1.125
7 1046.0 (No LPF) 0.17  1.125

*/
