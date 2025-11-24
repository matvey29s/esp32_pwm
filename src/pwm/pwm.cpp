#include "pwm.h"

PWMController::PWMController(uint8_t pin) : pin_(pin), dutyCycle_(0), 
                                           increasing_(true), lastLongPressTime_(0) {
}

void PWMController::begin() {
    // Настройка ШИМ для ESP32
    ledcSetup(0, 5000, 8);      // Канал 0, частота 5kHz, разрешение 8 бит
    ledcAttachPin(pin_, 0);     // Привязка пина к каналу 0
    setDutyCycle(0);            // Инициализация с 0%
    Logger::info("PWM initialized on pin " + String(pin_));
}

void PWMController::setDutyCycle(uint8_t dutyCycle) {
    if (dutyCycle > PWM_MAX) {
        dutyCycle = PWM_MAX;
    }
    
    dutyCycle_ = dutyCycle;
    updatePWM();
    Logger::info("PWM set to " + String(dutyCycle_) + "%");
}

uint8_t PWMController::getDutyCycle() const {
    return dutyCycle_;
}

void PWMController::increaseDutyCycle() {
    if (dutyCycle_ < PWM_MAX) {
        dutyCycle_ += PWM_STEP;
        if (dutyCycle_ > PWM_MAX) {
            dutyCycle_ = PWM_MAX;
        }
        updatePWM();
        Logger::info("PWM increased to " + String(dutyCycle_) + "%");
    }
}

void PWMController::decreaseDutyCycle() {
    if (dutyCycle_ > PWM_MIN) {
        dutyCycle_ -= PWM_STEP;
        if (dutyCycle_ < PWM_MIN) {
            dutyCycle_ = PWM_MIN;
        }
        updatePWM();
        Logger::info("PWM decreased to " + String(dutyCycle_) + "%");
    }
}

void PWMController::handleLongPress() {
    unsigned long currentTime = millis();
    
    if ((currentTime - lastLongPressTime_) >= PWM_LONG_PRESS_INTERVAL_MS) {
        cycleDutyCycle();
        lastLongPressTime_ = currentTime;
    }
}

void PWMController::cycleDutyCycle() {
    if (increasing_) {
        if (dutyCycle_ < PWM_MAX) {
            dutyCycle_ += PWM_STEP;
        } else {
            increasing_ = false;
            dutyCycle_ -= PWM_STEP;
        }
    } else {
        if (dutyCycle_ > PWM_MIN) {
            dutyCycle_ -= PWM_STEP;
        } else {
            increasing_ = true;
            dutyCycle_ += PWM_STEP;
        }
    }
    
    updatePWM();
    Logger::debug("Cyclic PWM: " + String(dutyCycle_) + "%");
}

void PWMController::resetLongPressCycle() {
    increasing_ = true;
    lastLongPressTime_ = 0;
}

void PWMController::updatePWM() {
    uint32_t pwmValue = (dutyCycle_ * 255) / 100;
    ledcWrite(0, pwmValue);
}