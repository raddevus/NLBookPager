/*
  Kindle Page Turner - ESP32 BLE HID Keyboard
  --------------------------------------------
  Uses HijelHID_BLEKeyboard (built on modern NimBLE-Arduino), which properly
  distinguishes isConnected() (raw BLE link) from isPaired() (fully
  authenticated and ready to receive HID input). This matters because a lot
  of BLE HID libraries report "connected" before pairing/bonding actually
  completes, which is why the iPad was never treating the device as a
  real keyboard.

  Library: HijelHID_BLEKeyboard
  Install via Arduino Library Manager: search "HijelHID"
  Requires: NimBLE-Arduino >= 2.3.8, ESP32 Arduino Core >= 3.3.7

  Wiring:
    NEXT button: GPIO 4 -> GND
    PREV button: GPIO 5 -> GND
  (Internal pull-ups used, no external resistors needed.)
*/

#include "HijelHID_BLEKeyboard.h"
#include <WiFi.h>
#include "esp_wifi.h"

HijelHID_BLEKeyboard keyboard("Kindle Page Turner", "DIY Electronics", 100);

const int NEXT_PIN = 4;
const int PREV_PIN = 5;
const int LOCK_SCREEN_PIN = 15;

const unsigned long DEBOUNCE_MS = 200;
unsigned long lastNextPress = 0;
unsigned long lastPrevPress = 0;

void onPairingComplete(bool success) {
  if (success) {
    Serial.println("Pairing successful - keyboard is ready.");
  } else {
    Serial.println("Pairing failed. Try removing and re-pairing on the iPad.");
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(NEXT_PIN, INPUT_PULLUP);
  pinMode(PREV_PIN, INPUT_PULLUP);
  pinMode(LOCK_SCREEN_PIN, INPUT_PULLUP);

  keyboard.onPairingComplete(onPairingComplete);

  Serial.println("Starting BLE keyboard...");
  keyboard.begin();
  WiFi.mode(WIFI_OFF);   // disable WiFi stack
  esp_wifi_stop();       // fully stop WiFi radio
}

void loop() {
  unsigned long now = millis();

  // isPaired() only returns true once the link is fully authenticated.
  // Sending input before that point has no effect.
  if (keyboard.isPaired()) {
    if (digitalRead(NEXT_PIN) == LOW && (now - lastNextPress) > DEBOUNCE_MS) {
      Serial.println("Next page");
      keyboard.tap(KEY_RIGHT);
      lastNextPress = now;
    }

    if (digitalRead(PREV_PIN) == LOW && (now - lastPrevPress) > DEBOUNCE_MS) {
      Serial.println("Previous page");
      keyboard.tap(KEY_LEFT);
      lastPrevPress = now;
    }
    if (digitalRead(LOCK_SCREEN_PIN) == LOW && (now - lastPrevPress) > DEBOUNCE_MS) {
      Serial.println("Lock the device");
      keyboard.tap(KEY_Q, KEY_MOD_LCTRL | KEY_MOD_LGUI);
      lastPrevPress = now;
    }
  }

  delay(20);
}
