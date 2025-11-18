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
    
    // Состояния конечного автомата
    enum State {
        STATE_IDLE,
        STATE_PRESSED,
        STATE_RELEASED,
        STATE_DOUBLE_WAIT,
        STATE_LONG_PRESS
    };
    
    State state_;
    State lastState_;
    unsigned long lastDebounceTime_;
    unsigned long pressStartTime_;
    unsigned long lastReleaseTime_;
    int clickCount_;
    bool stableState_;
    bool lastStableState_;
    
    bool readDebouncedButton();
    ButtonEvent handleStateMachine();
};

#endif