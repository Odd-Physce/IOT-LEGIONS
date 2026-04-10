#include <Wire.h>
#include <WiFi.h>
#include <esp_now.h>

#define MPU6050_ADDR 0x68

int16_t AccX, AccY, AccZ;
int16_t GyroX, GyroY, GyroZ;

uint8_t masterAddress[] = {0x00, 0x70, 0x07, 0x24, 0x2A, 0xD4}; // 🔥 put master MAC

struct SensorData {
  int node_id;
  float ax, ay, az;
  float gx, gy, gz;
};

SensorData data;

void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);

  // Wake MPU6050
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  WiFi.mode(WIFI_STA);

  esp_now_init();

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, masterAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  esp_now_add_peer(&peerInfo);

  data.node_id = 2; // 👉 CHANGE TO 2 for Node 2
}

void loop() {

  // Read sensor
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDR, 14, true);

  AccX = Wire.read() << 8 | Wire.read();
  AccY = Wire.read() << 8 | Wire.read();
  AccZ = Wire.read() << 8 | Wire.read();

  Wire.read(); Wire.read();

  GyroX = Wire.read() << 8 | Wire.read();
  GyroY = Wire.read() << 8 | Wire.read();
  GyroZ = Wire.read() << 8 | Wire.read();

  data.ax = AccX / 16384.0;
  data.ay = AccY / 16384.0;
  data.az = AccZ / 16384.0;

  data.gx = GyroX / 131.0;
  data.gy = GyroY / 131.0;
  data.gz = GyroZ / 131.0;

  // Send to master
  esp_now_send(masterAddress, (uint8_t *) &data, sizeof(data));

  delay(50); // 20Hz
}