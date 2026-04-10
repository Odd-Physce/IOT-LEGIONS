#include <WiFi.h>
#include <WebServer.h>

// Your WiFi
const char* ssid = "robot";
const char* password = "12345678";

WebServer server(80);

// Motor pins
#define ENA 27
#define IN1 14
#define IN2 13

#define ENB 15
#define IN3 4
#define IN4 2

// -------- MOTOR FUNCTIONS --------
void forward() {
  digitalWrite(ENA, HIGH);
  digitalWrite(ENB, HIGH);

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void backward() {
  digitalWrite(ENA, HIGH);
  digitalWrite(ENB, HIGH);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void left() {
  digitalWrite(ENA, HIGH);
  digitalWrite(ENB, HIGH);

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void right() {
  digitalWrite(ENA, HIGH);
  digitalWrite(ENB, HIGH);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void stopMotor() {
  digitalWrite(ENA, LOW);
  digitalWrite(ENB, LOW);
}

// -------- ROUTES --------
void handleForward() { forward(); server.send(200, "text/plain", "Forward"); }
void handleBack() { backward(); server.send(200, "text/plain", "Backward"); }
void handleLeft() { left(); server.send(200, "text/plain", "Left"); }
void handleRight() { right(); server.send(200, "text/plain", "Right"); }
void handleStop() { stopMotor(); server.send(200, "text/plain", "Stop"); }

void setup() {
  Serial.begin(115200);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Connect to WiFi
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());  // IMPORTANT

  // Routes
  server.on("/forward", handleForward);
  server.on("/back", handleBack);
  server.on("/left", handleLeft);
  server.on("/right", handleRight);
  server.on("/stop", handleStop);

  server.begin();
}

void loop() {
  server.handleClient();
}