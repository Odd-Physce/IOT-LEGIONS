#include <WiFi.h>
#include <esp_now.h>
#include <nani2203-project-1_inferencing.h>

// -------- CONFIG --------
#define MAX_SAMPLES (EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE / 6)

// -------- DATA STRUCTURE --------
struct SensorData {
  int node_id;
  float ax, ay, az;
  float gx, gy, gz;
};

// -------- BUFFERS --------
float buffer1[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
float buffer2[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];

int index1 = 0;
int index2 = 0;

// -------- INFERENCE FUNCTION --------
void runInference(float *buffer, int node_id) {

  signal_t signal;
  numpy::signal_from_buffer(buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);

  ei_impulse_result_t result;
  run_classifier(&signal, &result, false);

  float maxVal = 0;
  String label = "";

  // Find highest confidence label
  for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
    if (result.classification[i].value > maxVal) {
      maxVal = result.classification[i].value;
      label = result.classification[i].label;
    }
  }

  // -------- JSON OUTPUT (VERY IMPORTANT) --------
  Serial.print("{");

  Serial.print("\"node\":");
  Serial.print(node_id);
  Serial.print(",");

  Serial.print("\"label\":\"");
  Serial.print(label);
  Serial.print("\",");

  Serial.print("\"confidence\":");
  Serial.print(maxVal, 4);
  Serial.print(",");

  // Send latest sensor values (last sample)
  int last = (MAX_SAMPLES - 1) * 6;

  Serial.print("\"ax\":");
  Serial.print(buffer[last + 0], 4);
  Serial.print(",");

  Serial.print("\"ay\":");
  Serial.print(buffer[last + 1], 4);
  Serial.print(",");

  Serial.print("\"az\":");
  Serial.print(buffer[last + 2], 4);
  Serial.print(",");

  Serial.print("\"gx\":");
  Serial.print(buffer[last + 3], 4);
  Serial.print(",");

  Serial.print("\"gy\":");
  Serial.print(buffer[last + 4], 4);
  Serial.print(",");

  Serial.print("\"gz\":");
  Serial.print(buffer[last + 5], 4);
  Serial.print(",");

  // Timestamp (ms)
  Serial.print("\"time\":");
  Serial.print(millis());

  Serial.println("}");

  // -------- DEBUG PRINT --------
  Serial.print("Node ");
  Serial.print(node_id);
  Serial.print(" → ");
  Serial.print(label);
  Serial.print(" (");
  Serial.print(maxVal);
  Serial.println(")");
}

// -------- ESP-NOW CALLBACK --------
void onReceive(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {

  if (len != sizeof(SensorData)) return;

  SensorData data;
  memcpy(&data, incomingData, sizeof(data));

  // -------- NODE 1 --------
  if (data.node_id == 1) {

    buffer1[index1 * 6 + 0] = data.ax;
    buffer1[index1 * 6 + 1] = data.ay;
    buffer1[index1 * 6 + 2] = data.az;
    buffer1[index1 * 6 + 3] = data.gx;
    buffer1[index1 * 6 + 4] = data.gy;
    buffer1[index1 * 6 + 5] = data.gz;

    index1++;

    if (index1 >= MAX_SAMPLES) {
      runInference(buffer1, 1);
      index1 = 0;
    }
  }

  // -------- NODE 2 --------
  else if (data.node_id == 2) {

    buffer2[index2 * 6 + 0] = data.ax;
    buffer2[index2 * 6 + 1] = data.ay;
    buffer2[index2 * 6 + 2] = data.az;
    buffer2[index2 * 6 + 3] = data.gx;
    buffer2[index2 * 6 + 4] = data.gy;
    buffer2[index2 * 6 + 5] = data.gz;

    index2++;

    if (index2 >= MAX_SAMPLES) {
      runInference(buffer2, 2);
      index2 = 0;
    }
  }
}

// -------- SETUP --------
void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  Serial.println("Initializing ESP-NOW...");

  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ ESP-NOW Init Failed");
    return;
  }

  esp_now_register_recv_cb(onReceive);

  Serial.println("🔥 Master TinyML Ready");
}

// -------- LOOP --------
void loop() {
  // Nothing needed (event-based system)
}