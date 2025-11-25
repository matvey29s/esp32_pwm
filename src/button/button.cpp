#include "button.h"

Button::Button(uint8_t pin) : pin_(pin), currentState_(HIGH), lastState_(HIGH),
                             lastPressTime_(0), lastReleaseTime_(0),
                             lastDebounceTime_(0), clickCount_(0),
                             longPressEventSent_(false) {  // Инициализируем новое поле
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
                longPressEventSent_ = false;  // Сбрасываем флаг при новом нажатии
                Logger::debug("Button pressed");
            } else {
                // Кнопка отпущена
                lastReleaseTime_ = millis();
                clickCount_++;
                Logger::debug("Button released, click count: " + String(clickCount_));
                
                // Сбрасываем флаг длительного нажатия при отпускании
                longPressEventSent_ = false;
            }
        }
    }
    
    lastState_ = reading;
}

ButtonEvent Button::getEvent() {
    ButtonEvent event = EVENT_NONE;
    
    // Проверка длительного нажатия (пока кнопка нажата и еще не отправляли событие)
    if (currentState_ == LOW && !longPressEventSent_) {
        if (millis() - lastPressTime_ > LONG_PRESS_MIN_MS) {
            event = EVENT_LONG_PRESS;
            longPressEventSent_ = true; // Помечаем что отправили событие
            clickCount_ = 0; // Сбрасываем счетчик кликов
            Logger::info(">>> LONG PRESS EVENT <<<");
            return event;
        }
    }
    
    // Обработка кликов (только когда кнопка отпущена)
    if (currentState_ == HIGH && clickCount_ > 0) {
        if (millis() - lastReleaseTime_ > DOUBLE_CLICK_MAX_MS) {
            if (clickCount_ == 1) {
                event = EVENT_SINGLE_CLICK;
                Logger::info(">>> SINGLE CLICK EVENT <<<");
            } else if (clickCount_ >= 2) {
                event = EVENT_DOUBLE_CLICK;
                Logger::info(">>> DOUBLE CLICK EVENT <<<");
            }
            clickCount_ = 0;
        }
    }
    
    return event;
}

bool Button::isPressed() {
    return currentState_ == LOW;
}