#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Піни
const int ledPin = 9;
const int coarsePot = A0; // базова частота
const int finePot   = A1; // зсув

const int buttonPin = 2;

// Діапазон
const float minFreq = 1.0;
const float maxFreq = 1200.0;

// Стан
bool strobeEnabled = true;

// кнопка
bool lastButtonState = HIGH;
bool lastStableButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const int debounceDelay = 50;

// LCD
unsigned long lastDisplayUpdate = 0;
const int displayInterval = 120;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("STROBOSCOPE");
  lcd.setCursor(0, 1);
  lcd.print("READY");

  delay(1200);
  lcd.clear();
}

void loop() {

  // ======================
  // КНОПКА ON/OFF
  // ======================
  bool reading = digitalRead(buttonPin);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {

    if (reading != lastStableButtonState) {
      lastStableButtonState = reading;

      if (lastStableButtonState == LOW) {
        strobeEnabled = !strobeEnabled;
      }
    }
  }

  lastButtonState = reading;

  // ======================
  // ГРУБА частота
  // ======================
  int coarseValue = analogRead(coarsePot);

  float normalized = coarseValue / 1023.0;

  float baseFreq =
      minFreq * pow(maxFreq / minFreq, normalized);

  // ======================
  // ЗСУВ (fine pot)
  // ======================
  int fineValue = analogRead(finePot);

  float offset =
      map(fineValue, 0, 1023, -50, 50) / 10.0; // -5 .. +5 Hz

  float freq = baseFreq + offset;

  // обмеження
  if (freq < minFreq) freq = minFreq;
  if (freq > maxFreq) freq = maxFreq;

  // ======================
  // RPM
  // ======================
  unsigned long rpm = freq * 60.0;

  // ======================
  // період
  // ======================
  unsigned long period = 1000000.0 / freq;

  // duty
  unsigned long onTime;

  if (freq < 20) onTime = period / 3;
  else if (freq < 100) onTime = period / 5;
  else onTime = period / 10;

  unsigned long offTime = period - onTime;

  // ======================
  // LED
  // ======================
  if (strobeEnabled) {
    digitalWrite(ledPin, HIGH);
    delayMicroseconds(onTime);

    digitalWrite(ledPin, LOW);
    delayMicroseconds(offTime);
  } else {
    digitalWrite(ledPin, LOW);
    delay(5);
  }

  // ======================
  // LCD
  // ======================
  if (millis() - lastDisplayUpdate > displayInterval) {

    lastDisplayUpdate = millis();

    lcd.setCursor(0, 0);
    lcd.print(strobeEnabled ? "ON  Hz:" : "OFF Hz:");

    lcd.print((int)freq);
    lcd.print("   ");

    lcd.setCursor(0, 1);
    lcd.print("RPM:");
    lcd.print(rpm);
    lcd.print("    ");
  }
}