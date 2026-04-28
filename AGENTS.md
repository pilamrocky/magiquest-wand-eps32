# PROJECT BRIEF: MagiQuest Wand IR Reader

**Goal:** Create a firmware for an ESP32-WROOM-32 that detects "magic casts" from a MagiQuest wand and integrates with Home Assistant.

## Hardware Stack:
*   **MCU:** ESP32-WROOM-32.
*   **Sensor:** IR Receiver (38kHz) on GPIO 15.
*   **Actuator:** 2-pole Single LED on GPIO 2.
*   **Connectivity:** WiFi + MQTT.

## Software Requirements:
*   **Framework:** Arduino (C++) via VSCode/Antigravity.
*   **Libraries:** IRremote (v4.x+) for MagiQuest protocol, PubSubClient for MQTT.

## Logic:
*   Decode IR signals using the MAGIQUEST protocol.
*   On valid decode: Extract Wand ID and Magnitude.
*   Local Action: Flash the LED on GPIO 2.
*   Remote Action: Publish a JSON payload to MQTT topic `home/magiquest/cast`.

## Current Task: 
Initialize the project structure and implement the basic loop that prints the Wand ID to the Serial monitor.
