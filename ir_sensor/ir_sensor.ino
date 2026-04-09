#define IR_SENSOR 25

void setup() {
  Serial.begin(115200);
  pinMode(IR_SENSOR, INPUT);
}

void loop() {
  int sensorValue = digitalRead(IR_SENSOR);

  Serial.print("IR Value: ");
  Serial.println(sensorValue);

  if (sensorValue == LOW) {
    Serial.println("🚧 Object Detected!");
  } else {
    Serial.println("No Object");
  }

  Serial.println("-------------------");
  delay(500);
}