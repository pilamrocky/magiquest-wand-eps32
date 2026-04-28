# MagiQuest Wand to MQTT (Home Assistant) Bridge

This project is a custom firmware for the ESP32 that acts as a bridge between physical [MagiQuest](https://en.wikipedia.org/wiki/MagiQuest) wands and [Home Assistant](https://www.home-assistant.io/) (via MQTT). When you cast a spell with a wand, an infrared receiver picks up the signal, decodes the unique Wand ID, and publishes it as a JSON payload to your MQTT broker, allowing you to trigger smart home automations!

## Hardware Requirements
*   **Microcontroller:** ESP32 (e.g., ESP32-WROOM-32 DevKitC)
*   **Sensor:** 38kHz Infrared (IR) Receiver (e.g., VS1838B) connected to `GPIO 15`.
*   **Actuator/Feedback:** The built-in LED on `GPIO 2` (or an external LED) is used to provide visual feedback when a spell is cast and during the "cooldown" period.
*   **MagiQuest Wand:** Any standard IR-emitting MagiQuest wand.

## Software Requirements
*   **Development Environment:** [VSCode](https://code.visualstudio.com/) with the [PlatformIO IDE](https://platformio.org/) extension.
*   **Framework:** Arduino Core for ESP32.
*   **Dependencies:** (These are automatically installed by PlatformIO)
    *   `z3t0/IRremote` (v4.x+): Handles the extremely fast IR decoding (specifically using the `MAGIQUEST` protocol).
    *   `knolleary/PubSubClient`: Handles the MQTT connection.
    *   `bblanchon/ArduinoJson`: Formats the data into a neat JSON payload.

## Initial Setup & Configuration
1. **Clone the repository:**
   Download or clone this repository to your local machine and open the folder in VSCode.

2. **Set up Secrets:**
   You must provide your own WiFi and MQTT credentials. We use a local `secrets.h` file to keep these safe from version control.
   * Copy the `src/secrets.example.h` file and rename it to `src/secrets.h`.
   * Open `src/secrets.h` and fill in your specific details:
     ```c
     #define SECRET_WIFI_SSID "Your_WiFi_Name"
     #define SECRET_WIFI_PASSWORD "Your_WiFi_Password"
     #define SECRET_MQTT_SERVER "192.168.1.xxx"
     #define SECRET_MQTT_USER "your_mqtt_user"
     #define SECRET_MQTT_PASSWORD "your_mqtt_password"
     ```
   * *Note: `secrets.h` is added to `.gitignore` and will never be uploaded to GitHub.*

3. **Build and Upload:**
   Click the **Upload** button (the right-pointing arrow) in the PlatformIO toolbar at the bottom of VSCode. PlatformIO will automatically download the required libraries, compile the code, and flash the ESP32 over USB.

## How It Works
When the ESP32 boots, it connects to your WiFi and then to your MQTT broker. 
When you wave a MagiQuest wand at the IR receiver:
1. The ESP32 decodes the signal and extracts the `Wand ID` and the `Magnitude` of the cast.
2. It publishes a message to the MQTT topic `home/magiquest/cast` shaped like this:
   ```json
   {
     "wand_id": "0x6E03",
     "magnitude": 4
   }
   ```
3. **Cooldown Timer:** To prevent a single wave of the wand from rapidly firing dozens of MQTT messages, a non-blocking 5-second cooldown is triggered. The LED will turn solidly ON to let you know the system is "recharging." It will politely ignore subsequent signals until the LED turns off, at which point you can cast again.

## Integration with Home Assistant
You can use this MQTT topic to trigger automations in Home Assistant. For example, to toggle an office light when a specific wand is used:

```yaml
alias: "MagiQuest: Toggle Office Light"
trigger:
  - platform: mqtt
    topic: home/magiquest/cast
condition:
  - condition: template
    value_template: "{{ trigger.payload_json.wand_id == '0x6E03' }}"
action:
  - service: light.toggle
    target:
      entity_id: light.office_light
```

## Customizing the Code
If you want to edit the project to suit your needs, everything happens in `src/main.cpp`.
*   **Change the Cooldown Time:** Find `const unsigned long CAST_COOLDOWN_MS = 5000;` and change the number of milliseconds.
*   **Change the Pins:** Update `IR_RECEIVE_PIN` or `LED_PIN` at the top of the file to match your wiring.
*   **Change the MQTT Topic:** Edit `SECRET_MQTT_TOPIC` inside your `secrets.h` file.

Happy Casting! 🪄
