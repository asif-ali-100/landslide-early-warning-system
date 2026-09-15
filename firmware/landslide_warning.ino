/*
  Landslide / Disaster Early Warning Monitoring System
  -----------------------------------------------------
  Reads a tilt sensor and a rain gauge (analog), sends readings
  over Bluetooth (HC-05 / HC-06) to a companion smartphone app
  (built with MIT App Inventor) for real-time display and alerts.

  Hardware:
    - Arduino Uno / Nano (or compatible)
    - HC-05 / HC-06 Bluetooth module
    - Digital tilt switch sensor (e.g. SW-520D)
    - Analog rain sensor / rain gauge module

  Wiring:
    Tilt sensor signal   -> D2
    Rain sensor signal   -> A0
    HC-05/06 TXD         -> Arduino D10 (RX, via SoftwareSerial)
    HC-05/06 RXD         -> Arduino D11 (TX, via SoftwareSerial)
                            NOTE: HC-05/06 RXD is 3.3V logic.
                            Use a voltage divider (e.g. 1k/2k) or a
                            logic-level shifter between Arduino D11
                            and the module's RXD pin.
    HC-05/06 VCC         -> 5V (or 3.3V, check your module)
    HC-05/06 GND         -> GND

  Serial protocol sent over Bluetooth (one line per reading, ~1 Hz):
    "<tiltState>,<rainValue>"      e.g. "0,612"
    "ALERT"                        sent whenever a threshold is tripped

  License: MIT (see LICENSE file)
*/

#include <SoftwareSerial.h>

// ---------- Pin configuration ----------
const uint8_t PIN_BT_RX   = 10;   // Arduino RX  <- HC-05/06 TXD
const uint8_t PIN_BT_TX   = 11;   // Arduino TX  -> HC-05/06 RXD (via divider)
const uint8_t PIN_TILT    = 2;    // Digital tilt sensor
const uint8_t PIN_RAIN    = A0;   // Analog rain sensor

// ---------- Thresholds (calibrate for your sensors/site) ----------
// Most analog rain sensors output a LOWER value when wetter.
const int RAIN_THRESHOLD  = 500;
const unsigned long SAMPLE_INTERVAL_MS = 1000;

SoftwareSerial btSerial(PIN_BT_RX, PIN_BT_TX);

unsigned long lastSampleTime = 0;

void setup() {
  Serial.begin(9600);       // USB debug console
  btSerial.begin(9600);     // Default HC-05/06 baud rate
  pinMode(PIN_TILT, INPUT);

  Serial.println(F("Landslide Early Warning System - booting..."));
  btSerial.println(F("SYSTEM_READY"));
}

void loop() {
  unsigned long now = millis();
  if (now - lastSampleTime < SAMPLE_INTERVAL_MS) {
    return;
  }
  lastSampleTime = now;

  int tiltState = digitalRead(PIN_TILT);
  int rainValue = analogRead(PIN_RAIN);

  sendReading(tiltState, rainValue);

  if (isAlertCondition(tiltState, rainValue)) {
    triggerAlert();
  }
}

bool isAlertCondition(int tiltState, int rainValue) {
  return (tiltState == HIGH) || (rainValue < RAIN_THRESHOLD);
}

void sendReading(int tiltState, int rainValue) {
  String payload = String(tiltState) + "," + String(rainValue);
  btSerial.println(payload);
  Serial.println(payload);   // mirror to USB serial for debugging
}

void triggerAlert() {
  btSerial.println(F("ALERT"));
  Serial.println(F("ALERT: threshold exceeded!"));
}
