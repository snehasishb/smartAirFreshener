#include <WiFi.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include <esp_sleep.h>

#define RELAY_PIN 18

// Wi-Fi Credentials
const char* ssid = "SSID";
const char* password = "password";

// MQTT
const char* mqtt_server = "192.168.1.25";
const int mqtt_port = 1883;
const char* mqtt_user = "mqttuser";
const char* mqtt_pass = "hydrogen";
const char* mqtt_client_id = "ESP32PerfumeClient";

const char* topic_status = "home/perfumespray/status";
const char* topic_spray_count = "home/perfumespray/count";

// Time management
#define MAX_WIFI_RETRY_TIME 90000        // 1.5 minutes
#define MAX_MQTT_RETRY_TIME 90000        // 1.5 minutes
#define TOTAL_TIMEOUT_MS    180000       // Total 3 minutes max
#define SPRAY_INTERVAL_SEC  3600         // 1 hour deep sleep

WiFiClient espClient;
PubSubClient client(espClient);
Preferences prefs;

int sprayCount = 0;

// Trigger relay to spray
void triggerRelay() {
  Serial.println("Spraying perfume...");
  digitalWrite(RELAY_PIN, HIGH);
  delay(3000); // Spray duration
  digitalWrite(RELAY_PIN, LOW);
}

// Try Wi-Fi with timeout
bool connectWiFiWithTimeout() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < MAX_WIFI_RETRY_TIME) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    return true;
  } else {
    Serial.println("\nWiFi connection FAILED");
    return false;
  }
}

// Try MQTT with timeout
bool connectMQTTWithTimeout() {
  client.setServer(mqtt_server, mqtt_port);
  unsigned long start = millis();

  while (!client.connected() && millis() - start < MAX_MQTT_RETRY_TIME) {
    Serial.print("Attempting MQTT...");
    if (client.connect(mqtt_client_id, mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      return true;
    } else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      delay(1000);
    }
  }

  Serial.println("MQTT connection FAILED");
  return false;
}

// Publish spray status and count
void publishMQTT() {
  char countStr[10];
  itoa(sprayCount, countStr, 10);

  client.publish(topic_status, "ON");
  client.publish(topic_spray_count, countStr);
  delay(500);
  client.publish(topic_status, "OFF");
}

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  unsigned long startMillis = millis();

  // Restore spray count from flash
  prefs.begin("spray", false);
  sprayCount = prefs.getInt("sprayCount", 0);

  // Connect Wi-Fi
  bool wifiOK = connectWiFiWithTimeout();
  bool mqttOK = false;

  if (wifiOK) {
    mqttOK = connectMQTTWithTimeout();
  }

  // Spray anyway, whether or not MQTT works
  sprayCount++;
  prefs.putInt("sprayCount", sprayCount);
  triggerRelay();

  // If MQTT is okay, publish count
  if (mqttOK && client.connected()) {
    publishMQTT();
  }

  // Respect total timeout of 3 minutes
  unsigned long totalElapsed = millis() - startMillis;
  if (totalElapsed < TOTAL_TIMEOUT_MS) {
    delay(TOTAL_TIMEOUT_MS - totalElapsed);
  }

  Serial.println("Going to deep sleep for 1 hour...");
  esp_sleep_enable_timer_wakeup(SPRAY_INTERVAL_SEC * 1000000ULL);
  esp_deep_sleep_start();
}

void loop() {
  // unused – execution never returns from deep sleep
}
