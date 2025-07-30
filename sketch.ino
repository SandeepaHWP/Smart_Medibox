/**
  Combined Smart MediBox + DHT22 Temperature Monitoring

  Functions:
    1. Monitor LDR light intensity, send to Node-RED
    2. Control servo window to adjust brightness (with MQTT control)
    3. Periodically average and send light intensity
    4. Monitor DHT22 temperature, send to Node-RED
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include "DHTesp.h"

// WiFi & MQTT
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Pin Definitions
#define LIGHT_SENSOR_PIN 34
#define SERVO_PIN 27
#define DHT_PIN 15

// MQTT Topics
const char* MQTT_SERVER = "broker.hivemq.com";
const char* LIGHT_INTENSITY_TOPIC = "LIGHT_INTENSITY";
const char* SERVO_MOTOR_ANGLE_TOPIC = "SERVO_MOTOR_ANGLE";
const char* AVG_LIGHT_INTENSITY_TOPIC = "AVG_LIGHT_INTENSITY";
const char* TEMP_TOPIC = "PRABHASHANA-TEMP";

const char* SERVO_MIN_ANGLE_TOPIC = "SERVO_MIN_ANGLE";
const char* SERVO_CONTROL_FACTOR_TOPIC = "SERVO_CONTROLLING_FACTOR";
const char* SAMPLING_INTERVAL_TOPIC = "SAMPLING_INTERVAL";
const char* SENDING_INTERVAL_TOPIC = "SENDING_INTERVAL";
const char* IDEAL_TEMP_TOPIC = "IDEAL_TEMPERATURE";  // Added for Tmed

// Objects
Servo servo;
DHTesp dhtSensor;

// Variables
int offset_angle = 30;
float intensity = 0;
float control_factor = 0.75;
float Tmed = 30.0;  // Default ideal storage temperature (°C)

char intensityArr[6];
char servoAngleArr[10];
char tempAr[6];

unsigned long lastSampleTime = 0;
unsigned long lastSendTime = 0;
unsigned long lastTempSendTime = 0;

int sampling_interval = 5;
int sending_interval = 10;

#define MAX_SAMPLES 1440
float intensitySamples[MAX_SAMPLES];
int sampleCount = 0;

void setup() {
  Serial.begin(115200);
  setupWifi();
  setupMqtt();

  servo.attach(SERVO_PIN, 500, 2400);
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

  lastSampleTime = millis();
  lastSendTime = millis();
  lastTempSendTime = millis();
}

void loop() {
  if (!mqttClient.connected()) {
    connectToBroker();
  }

  mqttClient.loop();
  unsigned long currentTime = millis();

  // Sample light intensity
  if (currentTime - lastSampleTime >= sampling_interval * 1000) {
    lastSampleTime = currentTime;
    readAndBufferIntensity();
  }

  // Send average intensity
  if (currentTime - lastSendTime >= sending_interval * 1000) {
    lastSendTime = currentTime;
    sendAverageIntensity();
  }

  // Update temperature every second and update servo angle
  if (currentTime - lastTempSendTime >= 1000) {
    lastTempSendTime = currentTime;
    updateTemperature();
    mqttClient.publish(TEMP_TOPIC, tempAr);
    calculateServoPos();  // ✅ Fix: Update servo angle when temperature changes
  }

  delay(10);
}

// Setup Functions
void setupWifi() {
  WiFi.begin("Wokwi-GUEST", "", 6);
  Serial.print("Connecting to WIFI");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WIFI");
  Serial.println("IP: " + WiFi.localIP().toString());
}

void setupMqtt() {
  mqttClient.setServer(MQTT_SERVER, 1883);
  mqttClient.setCallback(receiveCallback);
}

void connectToBroker() {
  while (!mqttClient.connected()) {
    Serial.println("Attempting MQTT Connection....");
    if (mqttClient.connect("ESP32-CombinedBox")) {
      Serial.println("Connected");
      mqttClient.subscribe(SERVO_MIN_ANGLE_TOPIC);
      mqttClient.subscribe(SERVO_CONTROL_FACTOR_TOPIC);
      mqttClient.subscribe(SAMPLING_INTERVAL_TOPIC);
      mqttClient.subscribe(SENDING_INTERVAL_TOPIC);
      mqttClient.subscribe(IDEAL_TEMP_TOPIC);  // Subscribe to Tmed
    } else {
      Serial.println("Failed to connect, retrying...");
      delay(5000);
    }
  }
}

// Sensor Readings
void readAndBufferIntensity() {
  int analogValue = analogRead(LIGHT_SENSOR_PIN);
  float currentIntensity = map(analogValue, 0, 4095, 100, 0) / 100.00;

  if (sampleCount < MAX_SAMPLES) {
    intensitySamples[sampleCount++] = currentIntensity;
  } else {
    for (int i = 1; i < MAX_SAMPLES; i++) {
      intensitySamples[i - 1] = intensitySamples[i];
    }
    intensitySamples[MAX_SAMPLES - 1] = currentIntensity;
  }

  Serial.println("Buffered Intensity = " + String(currentIntensity));

  float prevValue = intensity;
  intensity = currentIntensity;
  String(intensity, 2).toCharArray(intensityArr, 6);
  mqttClient.publish(LIGHT_INTENSITY_TOPIC, intensityArr);

  if (prevValue != intensity) {
    calculateServoPos();
  }
}

void sendAverageIntensity() {
  if (sampleCount == 0) return;

  float sum = 0;
  for (int i = 0; i < sampleCount; i++) {
    sum += intensitySamples[i];
  }

  float avgIntensity = sum / sampleCount;
  Serial.println("Average Intensity = " + String(avgIntensity));

  char avgIntensityArr[6];
  String(avgIntensity, 2).toCharArray(avgIntensityArr, 6);
  mqttClient.publish(AVG_LIGHT_INTENSITY_TOPIC, avgIntensityArr);
  sampleCount = 0;
}

void updateTemperature() {
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  String(data.temperature, 2).toCharArray(tempAr, 6);
  Serial.println("Temperature = " + String(tempAr));
}

void calculateServoPos() {
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  float T = data.temperature;

  if (sampling_interval <= 0 || sending_interval <= 0 || T <= 0 || Tmed <= 0) {
    Serial.println("Invalid parameters for servo calculation.");
    return;
  }

  float lnRatio = -(log((float)sampling_interval / (float)sending_interval));
  float servo_angle = offset_angle + (180.0 - offset_angle) * intensity * control_factor * lnRatio * (T / Tmed);

  // Clamp angle
  servo_angle = constrain(servo_angle, 0, 180);
  servo.write(servo_angle);

  String(servo_angle, 2).toCharArray(servoAngleArr, 10);
  mqttClient.publish(SERVO_MOTOR_ANGLE_TOPIC, servoAngleArr);
  Serial.println("Servo Angle = " + String(servo_angle));
}

// MQTT Callback
void receiveCallback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Message received [" + String(topic) + "]");
  char payloadArr[length + 1];
  for (int i = 0; i < length; i++) {
    payloadArr[i] = (char) payload[i];
  }
  payloadArr[length] = '\0';

  if (strcmp(topic, SERVO_MIN_ANGLE_TOPIC) == 0) {
    offset_angle = atof(payloadArr);
    calculateServoPos();
  } else if (strcmp(topic, SERVO_CONTROL_FACTOR_TOPIC) == 0) {
    control_factor = atof(payloadArr);
    calculateServoPos();
  } else if (strcmp(topic, SAMPLING_INTERVAL_TOPIC) == 0) {
    sampling_interval = atoi(payloadArr);
  } else if (strcmp(topic, SENDING_INTERVAL_TOPIC) == 0) {
    sending_interval = atoi(payloadArr);
  } else if (strcmp(topic, IDEAL_TEMP_TOPIC) == 0) {
    Tmed = atof(payloadArr);
    calculateServoPos();
  }
}
