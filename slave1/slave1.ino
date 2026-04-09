/*
 * EDGE IMPULSE DATA COLLECTION (CLEAN VERSION)
 * Output: ax, ay, az, gx, gy, gz (CSV ONLY)
 * No extra text, no debug logs
 */

#include <Wire.h>
#include <math.h>

// ── CONFIG ─────────────────────────────────────────────
#define MPU6050_ADDR 0x68

#define ACCEL_SCALE 16384.0f
#define GYRO_SCALE 131.0f

#define FILTER_SIZE 10

#define GYRO_DEADZONE 0.5f
#define ACCEL_DEADZONE 0.02f

// ── STATE ─────────────────────────────────────────────
float axBuf[FILTER_SIZE] = {}, ayBuf[FILTER_SIZE] = {}, azBuf[FILTER_SIZE] = {};
float gxBuf[FILTER_SIZE] = {}, gyBuf[FILTER_SIZE] = {}, gzBuf[FILTER_SIZE] = {};
int filterIdx = 0;

// ── FUNCTIONS ─────────────────────────────────────────
float movAvg(float* buf, float val) {
  buf[filterIdx % FILTER_SIZE] = val;
  float sum = 0;
  for (int i = 0; i < FILTER_SIZE; i++) sum += buf[i];
  return sum / FILTER_SIZE;
}

float deadzone(float v, float t) {
  return (fabs(v) < t) ? 0 : v;
}

bool initMPU() {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x6B);
  Wire.write(0x00);
  return (Wire.endTransmission(true) == 0);
}

// ── SETUP ─────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(2000);

  Wire.begin(21, 22);
  Wire.setClock(400000);

  initMPU(); // no prints at all
}

// ── LOOP ─────────────────────────────────────────────
void loop() {

  // Read MPU6050
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);

  if (Wire.requestFrom(MPU6050_ADDR, 14, true) < 14) return;

  int16_t rAX = (Wire.read() << 8) | Wire.read();
  int16_t rAY = (Wire.read() << 8) | Wire.read();
  int16_t rAZ = (Wire.read() << 8) | Wire.read();
  Wire.read(); Wire.read();
  int16_t rGX = (Wire.read() << 8) | Wire.read();
  int16_t rGY = (Wire.read() << 8) | Wire.read();
  int16_t rGZ = (Wire.read() << 8) | Wire.read();

  float ax = rAX / ACCEL_SCALE;
  float ay = rAY / ACCEL_SCALE;
  float az = rAZ / ACCEL_SCALE;

  float gx = rGX / GYRO_SCALE;
  float gy = rGY / GYRO_SCALE;
  float gz = rGZ / GYRO_SCALE;

  // Deadzone
  ax = deadzone(ax, ACCEL_DEADZONE);
  ay = deadzone(ay, ACCEL_DEADZONE);
  az = deadzone(az, ACCEL_DEADZONE);

  gx = deadzone(gx, GYRO_DEADZONE);
  gy = deadzone(gy, GYRO_DEADZONE);
  gz = deadzone(gz, GYRO_DEADZONE);

  // Moving average filter
  ax = movAvg(axBuf, ax);
  ay = movAvg(ayBuf, ay);
  az = movAvg(azBuf, az);

  gx = movAvg(gxBuf, gx);
  gy = movAvg(gyBuf, gy);
  gz = movAvg(gzBuf, gz);

  filterIdx++;

  // ✅ PURE CSV OUTPUT (Edge Impulse friendly)
  Serial.print(ax, 5); Serial.print(",");
  Serial.print(ay, 5); Serial.print(",");
  Serial.print(az, 5); Serial.print(",");
  Serial.print(gx, 5); Serial.print(",");
  Serial.print(gy, 5); Serial.print(",");
  Serial.println(gz, 5);

  delay(20); // ~50Hz sampling
}