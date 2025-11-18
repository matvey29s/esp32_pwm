#include "pwm.h"

PWMController::PWMController(uint8_t pin) : pin_(pin), dutyCycle_(0), 
                                           increasing_(true), lastLongPressTime_(0) {
}

void PWMController::begin() {
    ledcSetup(0, 5000, 8); // канал 0, 5kHz, 8-bit resolution
    ledcAttachPin(pin_, 0); // привязываем пин к каналу
    setDutyCycle(0);
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
    } else {
        Logger::debug("PWM already at maximum");
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
    } else {
        Logger::debug("PWM already at minimum");
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
    // Конвертируем проценты в 8-битное значение (0-255)
    uint32_t pwmValue = (dutyCycle_ * 255) / 100;
    ledcWrite(0, pwmValue);
}