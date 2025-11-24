#include "button.h"

Button::Button(uint8_t pin) : pin_(pin), currentState_(HIGH), lastState_(HIGH),
                             lastPressTime_(0), lastReleaseTime_(0),
                             lastDebounceTime_(0), clickCount_(0),
                             longPressDetected_(false), longPressHandled_(false) {
}

void Button::begin() {
    pinMode(pin_, INPUT_PULLUP);
    Logger::info("Button initialized on pin " + String(pin_));
}

void Button::update() {
    bool reading = digitalRead(pin_);
    
    // Фильтрация дребезга
    if (reading != lastState_) {
        lastDebounceTime_ = millis();
    }
    
    if ((millis() - lastDebounceTime_) > DEBOUNCE_DELAY_MS) {
        if (reading != currentState_) {
            currentState_ = reading;
            
            if (currentState_ == LOW) {
                // Кнопка нажата
                lastPressTime_ = millis();
                longPressDetected_ = false;
                longPressHandled_ = false;
            } else {
                // Кнопка отпущена
                lastReleaseTime_ = millis();
                
                // Увеличиваем счетчик кликов только если не было длительного нажатия
                if (!longPressHandled_) {
                    clickCount_++;
                } else {
                    // После длительного нажатия сбрасываем счетчик
                    clickCount_ = 0;
                    longPressHandled_ = false;
                }
            }
        }
    }
    
    lastState_ = reading;
    
    // Проверка длительного нажатия
    if (currentState_ == LOW && !longPressDetected_) {
        if (millis() - lastPressTime_ > LONG_PRESS_MIN_MS) {
            longPressDetected_ = true;
            longPressHandled_ = true;  // Помечаем что длительное нажатие обработано
            clickCount_ = 0;  // Сбрасываем счетчик кликов
        }
    }
}

ButtonEvent Button::getEvent() {
    ButtonEvent event = EVENT_NONE;
    
    // Обработка длительного нажатия (сразу при обнаружении)
    if (longPressDetected_ && currentState_ == LOW) {
        event = EVENT_LONG_PRESS;
        longPressDetected_ = false;  // Сбрасываем флаг
        Logger::info("Button: LONG PRESS");
        return event;
    }
    
    // Обработка кликов (только если не было длительного нажатия)
    if (currentState_ == HIGH && clickCount_ > 0 && !longPressHandled_) {
        if (millis() - lastReleaseTime_ > DOUBLE_CLICK_MAX_MS) {
            if (clickCount_ == 1) {
                event = EVENT_SINGLE_CLICK;
                Logger::info("Button: SINGLE CLICK");
            } else if (clickCount_ >= 2) {
                event = EVENT_DOUBLE_CLICK;
                Logger::info("Button: DOUBLE CLICK");
            }
            clickCount_ = 0;
        }
    }
    
    return event;
}

bool Button::isPressed() {
    return currentState_ == LOW;
}