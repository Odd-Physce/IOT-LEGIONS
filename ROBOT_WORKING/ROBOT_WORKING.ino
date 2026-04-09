#define ENA 27
#define IN1 14
#define IN2 13

#define ENB 15
#define IN3 4
#define IN4 2

void setup() {
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  Serial.begin(115200);
}

void loop() {

  Serial.println("FULL SPEED FORWARD 🚀");

  digitalWrite(ENA, HIGH);
  digitalWrite(ENB, HIGH);

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  delay(5000);

  Serial.println("FULL SPEED BACKWARD 🔄");

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  delay(5000);

  Serial.println("SPIN LEFT ⚡");

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  delay(4000);

  Serial.println("SPIN RIGHT ⚡");

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  delay(4000);

  Serial.println("STOP");

  digitalWrite(ENA, LOW);
  digitalWrite(ENB, LOW);

  delay(3000);
}