#ifndef PWM_H
#define PWM_H

#include <Arduino.h>
#include "../common/config.h"
#include "../common/logger.h"

class PWMController {
public:
    PWMController(uint8_t pin);
    void begin();
    void setDutyCycle(uint8_t dutyCycle);
    uint8_t getDutyCycle() const;
    void increaseDutyCycle();
    void decreaseDutyCycle();
    void handleLongPress();
    void resetLongPressCycle();
    void cycleDutyCycle(); // Новый метод для циклического изменения

private:
    uint8_t pin_;
    uint8_t dutyCycle_;
    bool increasing_;
    unsigned long lastLongPressTime_;
    
    void updatePWM();
};

#endif