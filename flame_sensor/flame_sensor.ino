#define FLAME_SENSOR 26

void setup() {
  Serial.begin(115200);
  pinMode(FLAME_SENSOR, INPUT);
}

void loop() {
  int flameState = digitalRead(FLAME_SENSOR);

  if (flameState == LOW) {
    Serial.println("🔥 Flame Detected!");
  } else {
    Serial.println("No Flame");
  }

  delay(500);
}