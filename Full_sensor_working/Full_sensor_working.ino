#define IR_SENSOR 25
#define FLAME_SENSOR 26
#define TRIG_PIN 21
#define ECHO_PIN 22
#define MQ2_PIN 34

// Gas thresholds (based on your readings)
#define GAS_CLEAN 380
#define GAS_LOW 420

void setup() {
  Serial.begin(115200);

  pinMode(IR_SENSOR, INPUT);
  pinMode(FLAME_SENSOR, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  analogSetAttenuation(ADC_11db);
}

// -------- Ultrasonic Function --------
float getDistance() {
  long duration;

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH, 30000);

  float distance = duration * 0.034 / 2;
  return distance;
}

// -------- Gas Averaging --------
int getGasValue() {
  int sum = 0;

  for (int i = 0; i < 10; i++) {
    sum += analogRead(MQ2_PIN);
    delay(5);
  }

  return sum / 10;
}

void loop() {

  // ---- Read Sensors ----
  int irValue = digitalRead(IR_SENSOR);
  int flameValue = digitalRead(FLAME_SENSOR);
  int gasValue = getGasValue();
  float distance = getDistance();

  Serial.println("===== SENSOR DATA =====");

  // ---- IR ----
  Serial.print("IR: ");
  if (irValue == LOW)
    Serial.println("🚧 Object Detected");
  else
    Serial.println("No Object");

  // ---- Flame ----
  Serial.print("Flame: ");
  if (flameValue == LOW)
    Serial.println("🔥 Detected");
  else
    Serial.println("No Flame");

  // ---- Gas ----
  Serial.print("Gas Value: ");
  Serial.print(gasValue);

  if (gasValue < GAS_CLEAN) {
    Serial.println(" → ✅ Clean Air");
  }
  else if (gasValue < GAS_LOW) {
    Serial.println(" → ⚠️ Slight Gas");
  }
  else {
    Serial.println(" → 🚨 HIGH GAS");
  }

  // ---- Distance ----
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // -------- SMART LOGIC --------
  if (flameValue == LOW && gasValue > GAS_LOW) {
    Serial.println("🚨🔥 FIRE + GAS EMERGENCY!");
  }
  else if (gasValue > GAS_LOW) {
    Serial.println("⚠️ GAS LEAK DETECTED!");
  }
  else if (distance < 20 || irValue == LOW) {
    Serial.println("🚧 OBSTACLE DETECTED!");
  }
  else {
    Serial.println("✅ ALL NORMAL");
  }

  Serial.println("========================\n");

  delay(1000);
}