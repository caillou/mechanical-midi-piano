#include <Arduino.h>

// Morse code timing constants (in milliseconds)
// Base unit: 200ms for readable blink speed
const unsigned int DIT_DURATION = 200;   // Dot: 1 unit
const unsigned int DAH_DURATION = 600;   // Dash: 3 units
const unsigned int ELEMENT_GAP = 200;    // Gap between elements: 1 unit
const unsigned int LETTER_GAP = 600;     // Gap between letters: 3 units
const unsigned int WORD_GAP = 1400;      // Gap between SOS cycles: 7 units

void dot() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(DIT_DURATION);
    digitalWrite(LED_BUILTIN, LOW);
    delay(ELEMENT_GAP);
}

void dash() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(DAH_DURATION);
    digitalWrite(LED_BUILTIN, LOW);
    delay(ELEMENT_GAP);
}

void letterS() {
    dot();
    dot();
    dot();
}

void letterO() {
    dash();
    dash();
    dash();
}

void sos() {
    letterS();
    delay(LETTER_GAP - ELEMENT_GAP);  // Adjust for already-added element gap
    letterO();
    delay(LETTER_GAP - ELEMENT_GAP);
    letterS();
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    sos();
    delay(WORD_GAP);
}
