#include "led.h"

LED::LED(uint8_t pin) : pin_(pin), state_(false) {
}

void LED::begin() {
    pinMode(pin_, OUTPUT);
    setState(false);
    Logger::info("LED initialized on pin " + String(pin_));
}

void LED::setState(bool state) {
    state_ = state;
    digitalWrite(pin_, state_ ? HIGH : LOW);
}

void LED::toggle() {
    setState(!state_);
}