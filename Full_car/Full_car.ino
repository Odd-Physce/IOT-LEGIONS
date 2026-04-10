#include <WiFi.h>

// -------- WiFi --------
const char* ssid = "robot";
const char* password = "12345678";

WiFiServer server(80);

// -------- Motor Pins --------
#define ENA 27
#define IN1 14
#define IN2 13

#define ENB 15
#define IN3 4
#define IN4 2

// -------- Sensor Pins --------
#define IR_SENSOR 25
#define FLAME_SENSOR 26
#define MQ2_PIN 34

// -------- Queue --------
QueueHandle_t commandQueue;

// -------- GLOBAL SENSOR VALUES --------
int irValue = 0;
int flameValue = 0;
int gasValue = 0;

// -------- MOTOR CONTROL --------
void executeCommand(String cmd) {
  if (cmd == "forward") {
    digitalWrite(ENA, HIGH);
    digitalWrite(ENB, HIGH);
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  }
  else if (cmd == "back") {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  }
  else if (cmd == "left") {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  }
  else if (cmd == "right") {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  }
  else {
    digitalWrite(ENA, LOW);
    digitalWrite(ENB, LOW);
  }
}

// -------- CORE 0: SENSOR + MOTOR TASK --------
void sensorTask(void *pvParameters) {

  String receivedCommand = "stop";

  while (true) {

    // -------- Read sensors --------
    irValue = digitalRead(IR_SENSOR);
    flameValue = digitalRead(FLAME_SENSOR);
    gasValue = analogRead(MQ2_PIN);

    Serial.print("IR: "); Serial.print(irValue);
    Serial.print(" | Flame: "); Serial.print(flameValue);
    Serial.print(" | Gas: "); Serial.println(gasValue);

    // -------- Receive command --------
    if (xQueueReceive(commandQueue, &receivedCommand, 0) == pdTRUE) {
      Serial.print("New Command: ");
      Serial.println(receivedCommand);
    }

    // -------- Safety Logic --------
    if (flameValue == LOW && gasValue > 420) {
      receivedCommand = "stop";
      Serial.println("🚨 EMERGENCY STOP");
    }

    // -------- Execute --------
    executeCommand(receivedCommand);

    delay(200);
  }
}

// -------- CORE 1: WIFI TASK --------
void wifiTask(void *pvParameters) {

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial.println(WiFi.localIP());
  server.begin();

  while (true) {

    WiFiClient client = server.available();

    if (client) {
      String request = client.readStringUntil('\r');
      String cmd = "stop";

      if (request.indexOf("/forward") != -1) cmd = "forward";
      else if (request.indexOf("/back") != -1) cmd = "back";
      else if (request.indexOf("/left") != -1) cmd = "left";
      else if (request.indexOf("/right") != -1) cmd = "right";
      else if (request.indexOf("/stop") != -1) cmd = "stop";

      // -------- Send command to queue --------
      xQueueSend(commandQueue, &cmd, portMAX_DELAY);

      // -------- SEND SENSOR DATA OVER WIFI --------
      String sensorData = "IR=" + String(irValue) +
                          ",Flame=" + String(flameValue) +
                          ",Gas=" + String(gasValue);

      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/plain");
      client.println("Connection: close");
      client.println();
      client.println(sensorData);

      client.stop();
    }

    delay(10);
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

  // Sensor pins
  pinMode(IR_SENSOR, INPUT);
  pinMode(FLAME_SENSOR, INPUT);

  // -------- Create Queue --------
  commandQueue = xQueueCreate(5, sizeof(String));

  // -------- Tasks --------
  xTaskCreatePinnedToCore(sensorTask, "Sensor Task", 10000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(wifiTask, "WiFi Task", 10000, NULL, 1, NULL, 1);
}

void loop() {}