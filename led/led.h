#ifndef LED_H
#define LED_H

#include <Arduino.h>
#include "../common/logger.h"

class LED {
public:
    LED(uint8_t pin);
    void begin();
    void setState(bool state);
    void toggle();

private:
    uint8_t pin_;
    bool state_;
};

#endif