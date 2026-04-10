#include <WiFi.h>
#include <WiFiUdp.h>

// -------- WIFI --------
const char* ssid = "robot";
const char* password = "12345678";

// -------- UDP --------
WiFiUDP udp;
const int udpPort = 4210;
char incomingPacket[10];

// -------- MOTOR PINS --------
#define ENA 27
#define IN1 14
#define IN2 13

#define ENB 15
#define IN3 4
#define IN4 2

// -------- COMMAND --------
volatile char currentCommand = 's';

// -------- MOTOR TASK (CORE 1) --------
void motorTask(void * parameter) {
  while (true) {

    switch (currentCommand) {

      case 'f': // forward
        digitalWrite(ENA, HIGH);
        digitalWrite(ENB, HIGH);
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
        digitalWrite(IN3, HIGH);
        digitalWrite(IN4, LOW);
        break;

      case 'b': // backward
        digitalWrite(ENA, HIGH);
        digitalWrite(ENB, HIGH);
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, HIGH);
        break;

      case 'l': // left
        digitalWrite(ENA, HIGH);
        digitalWrite(ENB, HIGH);
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, HIGH);
        break;

      case 'r': // right
        digitalWrite(ENA, HIGH);
        digitalWrite(ENB, HIGH);
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
        digitalWrite(IN3, HIGH);
        digitalWrite(IN4, LOW);
        break;

      default: // stop
        digitalWrite(ENA, LOW);
        digitalWrite(ENB, LOW);
        break;
    }

    delay(5);
  }
}

// -------- UDP TASK (CORE 0) --------
void udpTask(void * parameter) {
  while (true) {

    int packetSize = udp.parsePacket();

    if (packetSize) {
      int len = udp.read(incomingPacket, 10);

      if (len > 0) {
        incomingPacket[len] = 0;
      }

      Serial.print("Received: ");
      Serial.println(incomingPacket);

      // Take only first character
      currentCommand = incomingPacket[0];
    }

    delay(5);
  }
}

void setup() {
  Serial.begin(115200);

  // Motor pins
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Connect WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("ESP IP: ");
  Serial.println(WiFi.localIP());

  // Start UDP
  udp.begin(udpPort);

  // Start Tasks
  xTaskCreatePinnedToCore(motorTask, "MotorTask", 10000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(udpTask, "UDPTask", 10000, NULL, 1, NULL, 0);
}

void loop() {
  // Nothing here
}