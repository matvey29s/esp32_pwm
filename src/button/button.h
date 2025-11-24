#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>
#include "../common/config.h"
#include "../common/logger.h"

enum ButtonEvent {
    EVENT_NONE,
    EVENT_SINGLE_CLICK, 
    EVENT_DOUBLE_CLICK,
    EVENT_LONG_PRESS
};

class Button {
public:
    Button(uint8_t pin);
    void begin();
    void update();
    ButtonEvent getEvent();
    bool isPressed();

private:
    uint8_t pin_;
    bool currentState_;
    bool lastState_;
    unsigned long lastPressTime_;
    unsigned long lastReleaseTime_;
    unsigned long lastDebounceTime_;
    int clickCount_;
    bool longPressDetected_;
    bool longPressHandled_;  // Новый флаг для отслеживания обработки длительного нажатия
};

#endif