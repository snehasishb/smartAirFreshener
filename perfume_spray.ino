#include <WiFi.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include <ArduinoOTA.h>

#define RELAY_PIN 18  // GPIO18 controls relay (ACTIVE HIGH)

// WiFi credentials
const char* ssid = "YOUR SSID";
const char* password = "YOUR PASSWORD";

// MQTT Broker (IP address of Home Assistant)
const char* mqtt_server = "192.168.1.25";
const int mqtt_port = 1883;
const char* mqtt_user = "mqttuser";
const char* mqtt_pass = "password";
const char* mqtt_client_id = "ESP32PerfumeClient";

// MQTT Topics
const char* topic_control = "home/perfumespray/control";
const char* topic_status = "home/perfumespray/status";
const char* topic_spray_count = "home/perfumespray/count";

// Timing
unsigned long lastTrigger = 0;
const unsigned long sprayInterval = 3600000UL;  // 1 hour

// Spray count (stored in non-volatile memory)
Preferences prefs;
int sprayCount = 0;

WiFiClient espClient;
PubSubClient client(espClient);

void triggerRelay() {
  Serial.println("Spraying perfume...");
  digitalWrite(RELAY_PIN, HIGH);   // Relay ON
  client.publish(topic_status, "ON");
  delay(3000);
  digitalWrite(RELAY_PIN, LOW);    // Relay OFF
  client.publish(topic_status, "OFF");

  // Update spray count
  sprayCount++;
  prefs.putInt("sprayCount", sprayCount);

  // Publish updated count
  char countStr[10];
  itoa(sprayCount, countStr, 10);
  client.publish(topic_spray_count, countStr);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  Serial.printf("MQTT message received [%s]: %s\n", topic, msg.c_str());

  if (String(topic) == topic_control && msg == "ON") {
    triggerRelay();
  }
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect(mqtt_client_id, mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      client.subscribe(topic_control);
    } else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void setupOTA() {
  ArduinoOTA.setHostname("perfumespray");
  ArduinoOTA.begin();
}

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);  // OFF by default (active HIGH relay)

  WiFi.setHostname("perfumespray");
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());

  setupOTA();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);

  prefs.begin("spray", false);
  sprayCount = prefs.getInt("sprayCount", 0);
  Serial.printf("Spray count (restored): %d\n", sprayCount);
}

void loop() {
  ArduinoOTA.handle();

  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  unsigned long currentMillis = millis();
  if (currentMillis - lastTrigger >= sprayInterval) {
    triggerRelay();
    lastTrigger = currentMillis;
  }
}
