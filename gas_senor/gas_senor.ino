#define MQ2_PIN 34

void setup() {
  Serial.begin(115200);
}

void loop() {
  int value = analogRead(MQ2_PIN);

  Serial.print("Gas Level: ");
  Serial.print(value);

  if (value < 1000) {
    Serial.println(" → Clean Air");
  } else if (value < 2000) {
    Serial.println(" → Slight Gas");
  } else {
    Serial.println(" → HIGH GAS ⚠️");
  }

  delay(300);
}