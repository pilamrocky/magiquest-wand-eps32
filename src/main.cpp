#include <Arduino.h>

// Explicitly enable only MagiQuest decoding to save memory and processing
#define DECODE_MAGIQUEST
#include <IRremote.hpp>

// Hardware pins
const int IR_RECEIVE_PIN = 15;
const int LED_PIN = 2;

void setup() {
    // Initialize Serial monitor
    Serial.begin(115200);
    // Wait a brief moment for the serial monitor to attach
    delay(1000);

    Serial.println("\n--- MagiQuest Wand IR Reader ---");

    // Initialize the LED pin
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // Start the IR receiver
    // ENABLE_LED_FEEDBACK will flash the built-in LED (if defined) when IR signals are received
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
    
    Serial.print("Ready to receive IR signals on GPIO ");
    Serial.println(IR_RECEIVE_PIN);
}

void loop() {
    // Check if an IR signal has been received and decoded
    if (IrReceiver.decode()) {
        // Check if the received signal is from a MagiQuest wand
        if (IrReceiver.decodedIRData.protocol == MAGIQUEST) {
            // MagiQuest protocol typically uses:
            // address: Wand ID
            // command: Magnitude (the force/type of the cast)
            uint32_t wandID = IrReceiver.decodedIRData.address;
            uint16_t magnitude = IrReceiver.decodedIRData.command;
            
            Serial.print("Valid Cast Detected! -> ");
            Serial.print("Wand ID: 0x");
            Serial.print(wandID, HEX); // Print as hexadecimal
            Serial.print(" | Magnitude: ");
            Serial.println(magnitude);
            
            // Perform the local action: Flash the LED
            digitalWrite(LED_PIN, HIGH);
            delay(100); // 100ms flash
            digitalWrite(LED_PIN, LOW);
        } else {
            // Optional: for debugging, you can print other signals that get picked up
            // Serial.print("Ignored Protocol: ");
            // Serial.println(IrReceiver.decodedIRData.protocol);
        }
        
        // Resume receiving the next IR signal
        IrReceiver.resume();
    }
}
