#include "secrets.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>


// Explicitly enable only MagiQuest decoding to save memory and processing
#define DECODE_MAGIQUEST
#include <IRremote.hpp>

// Hardware pins
const int IR_RECEIVE_PIN = 15;
const int LED_PIN = 2;

// --- Configuration ---
// WiFi settings
const char *ssid = SECRET_WIFI_SSID;
const char *password = SECRET_WIFI_PASSWORD;

// MQTT settings
const char *mqtt_server = SECRET_MQTT_SERVER;
const int mqtt_port = SECRET_MQTT_PORT;
const char *mqtt_user = SECRET_MQTT_USER;
const char *mqtt_password = SECRET_MQTT_PASSWORD;
const char *mqtt_topic = SECRET_MQTT_TOPIC;

// Cooldown settings
const unsigned long CAST_COOLDOWN_MS = 5000; // 5 second cooldown
unsigned long lastCastTime = 0;
bool isCooldownActive = false;

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32MagiQuest-";
    clientId += String(random(0xffff), HEX);

    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected to MQTT broker!");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" - trying again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  // Initialize Serial monitor
  Serial.begin(115200);
  // Wait a brief moment for the serial monitor to attach
  delay(1000);

  Serial.println("\n--- MagiQuest Wand IR Reader ---");

  // Initialize the LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Setup WiFi and MQTT
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);

  // Start the IR receiver
  // Using DISABLE_LED_FEEDBACK so we have full manual control of the LED for
  // the cooldown state
  IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK);

  Serial.print("Ready to receive IR signals on GPIO ");
  Serial.println(IR_RECEIVE_PIN);
}

void loop() {
  // Ensure MQTT is connected
  if (!client.connected()) {
    reconnect();
  }
  // Process incoming MQTT messages and maintain connection
  client.loop();

  // Manage the cooldown state and LED
  if (isCooldownActive) {
    if (millis() - lastCastTime >= CAST_COOLDOWN_MS) {
      // Cooldown is over, turn off the LED and accept new casts
      digitalWrite(LED_PIN, LOW);
      isCooldownActive = false;
      Serial.println("Wand cooldown finished. Ready for next cast.");
    }
  }

  // Check if an IR signal has been received and decoded
  if (IrReceiver.decode()) {
    // Check if the received signal is from a MagiQuest wand
    if (IrReceiver.decodedIRData.protocol == MAGIQUEST) {
      // Only process the cast if we are NOT in the cooldown period
      if (!isCooldownActive) {
        uint32_t wandID = IrReceiver.decodedIRData.address;
        uint16_t magnitude = IrReceiver.decodedIRData.command;

        Serial.print("Valid Cast Detected! -> ");
        Serial.print("Wand ID: 0x");
        Serial.print(wandID, HEX); // Print as hexadecimal
        Serial.print(" | Magnitude: ");
        Serial.println(magnitude);

        // Start cooldown: Turn on the LED to indicate we are "busy" processing
        // a spell
        isCooldownActive = true;
        lastCastTime = millis();
        digitalWrite(LED_PIN, HIGH);

        // Remote Action: Publish JSON to MQTT
        JsonDocument doc;
        char wandIdHex[16];
        // Format wand ID as hex string with 0x prefix
        snprintf(wandIdHex, sizeof(wandIdHex), "0x%lX", (unsigned long)wandID);

        doc["wand_id"] = wandIdHex;
        doc["magnitude"] = magnitude;

        char jsonBuffer[256];
        serializeJson(doc, jsonBuffer);

        client.publish(mqtt_topic, jsonBuffer);
        Serial.print("Published to MQTT: ");
        Serial.println(jsonBuffer);
      }
      // If isCooldownActive is true, we just ignore this extra IR signal from
      // the same swipe
    } else {
      // Optional: for debugging, you can print other signals that get picked up
      // Serial.print("Ignored Protocol: ");
      // Serial.println(IrReceiver.decodedIRData.protocol);
    }

    // Resume receiving the next IR signal
    IrReceiver.resume();
  }
}
