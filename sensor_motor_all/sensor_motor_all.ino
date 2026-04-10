#include <WiFi.h>
#include <WiFiUdp.h>

// -------- WIFI --------
const char* ssid = "robot";
const char* password = "12345678";

// -------- UDP --------
WiFiUDP udp;
WiFiUDP udpSend;

const int recvPort = 4210;
const int sendPort = 4211;

IPAddress pythonIP;

// -------- MOTOR PINS --------
#define ENA 27
#define IN1 14
#define IN2 13
#define ENB 15
#define IN3 4
#define IN4 2

// -------- SENSOR PINS --------
#define IR_SENSOR 25
#define FLAME_SENSOR 26
#define TRIG_PIN 21
#define ECHO_PIN 22
#define MQ2_PIN 34

volatile char currentCommand = 's';

// -------- MOTOR TASK (HIGHEST PRIORITY) --------
void motorTask(void * parameter) {
  while (true) {

    switch (currentCommand) {
      case 'f':
        digitalWrite(ENA, HIGH); digitalWrite(ENB, HIGH);
        digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
        digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
        break;

      case 'b':
        digitalWrite(ENA, HIGH); digitalWrite(ENB, HIGH);
        digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
        digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
        break;

      case 'l':
        digitalWrite(ENA, HIGH); digitalWrite(ENB, HIGH);
        digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
        digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
        break;

      case 'r':
        digitalWrite(ENA, HIGH); digitalWrite(ENB, HIGH);
        digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
        digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
        break;

      default:
        digitalWrite(ENA, LOW); digitalWrite(ENB, LOW);
        break;
    }

    delay(1);  // 🔥 ultra fast
  }
}

// -------- UDP RECEIVE (FAST) --------
void udpTask(void * parameter) {
  char incoming[10];

  while (true) {

    int size = udp.parsePacket();

    if (size) {
      pythonIP = udp.remoteIP();

      int len = udp.read(incoming, 10);
      if (len > 0) incoming[len] = 0;

      currentCommand = incoming[0];
    }

    delay(1);
  }
}

// -------- SENSOR TASK (LOW PRIORITY) --------
void sensorTask(void * parameter) {

  unsigned long lastSend = 0;

  while (true) {

    if (millis() - lastSend > 300) {   // 🔥 slow update (no lag)
      lastSend = millis();

      int ir = digitalRead(IR_SENSOR);
      int flame = digitalRead(FLAME_SENSOR);
      int gas = analogRead(MQ2_PIN);

      // ⚠️ TEMPORARILY REMOVE ULTRASONIC (MAIN LAG SOURCE)
      float dist = 0;

      String data = String(ir) + "," + String(flame) + "," +
                    String(gas) + "," + String(dist);

      if (pythonIP) {
        udpSend.beginPacket(pythonIP, sendPort);
        udpSend.print(data);
        udpSend.endPacket();
      }
    }

    delay(10); // very low priority
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  pinMode(IR_SENSOR, INPUT);
  pinMode(FLAME_SENSOR, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  Serial.println(WiFi.localIP());

  udp.begin(recvPort);

  // 🔥 PRIORITY SETTING
  xTaskCreatePinnedToCore(motorTask, "motor", 10000, NULL, 3, NULL, 1); // HIGH
  xTaskCreatePinnedToCore(udpTask, "udp", 10000, NULL, 2, NULL, 0);     // MEDIUM
  xTaskCreatePinnedToCore(sensorTask, "sensor", 10000, NULL, 1, NULL, 1); // LOW
}

void loop() {}