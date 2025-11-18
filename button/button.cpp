#include "button.h"

Button::Button(uint8_t pin) : pin_(pin), state_(STATE_IDLE), lastState_(STATE_IDLE),
                             lastDebounceTime_(0), pressStartTime_(0), lastReleaseTime_(0),
                             clickCount_(0), stableState_(HIGH), lastStableState_(HIGH) {
}

void Button::begin() {
    pinMode(pin_, INPUT_PULLUP);
    Logger::info("Button initialized on pin " + String(pin_));
}

void Button::update() {
    // Чтение кнопки с фильтрацией дребезга
    bool currentStableState = readDebouncedButton();
    
    // Обновление состояния конечного автомата только при изменении стабильного состояния
    if (currentStableState != lastStableState_) {
        if (currentStableState == LOW) { // Кнопка нажата (LOW из-за INPUT_PULLUP)
            state_ = STATE_PRESSED;
            pressStartTime_ = millis();
        } else { // Кнопка отпущена
            state_ = STATE_RELEASED;
            lastReleaseTime_ = millis();
        }
        lastStableState_ = currentStableState;
    }
    
    // Обработка таймаутов в состояниях
    unsigned long currentTime = millis();
    
    switch (state_) {
        case STATE_PRESSED:
            // Проверка на длительное нажатие
            if ((currentTime - pressStartTime_) >= LONG_PRESS_MIN_MS) {
                state_ = STATE_LONG_PRESS;
                clickCount_ = 0; // Сбрасываем счетчик кликов при длительном нажатии
            }
            break;
            
        case STATE_RELEASED:
            // Ожидание второго клика
            if ((currentTime - lastReleaseTime_) >= DOUBLE_CLICK_MAX_MS) {
                state_ = STATE_IDLE;
            } else {
                state_ = STATE_DOUBLE_WAIT;
            }
            break;
            
        case STATE_DOUBLE_WAIT:
            // Таймаут для двойного клика
            if ((currentTime - lastReleaseTime_) >= DOUBLE_CLICK_MAX_MS) {
                state_ = STATE_IDLE;
            }
            break;
            
        case STATE_LONG_PRESS:
            // Ожидаем отпускания кнопки
            if (currentStableState == HIGH) {
                state_ = STATE_IDLE;
            }
            break;
            
        case STATE_IDLE:
        default:
            break;
    }
}

ButtonEvent Button::getEvent() {
    return handleStateMachine();
}

bool Button::isPressed() {
    return stableState_ == LOW;
}

bool Button::readDebouncedButton() {
    bool reading = digitalRead(pin_);
    
    if (reading != lastStableState_) {
        lastDebounceTime_ = millis();
    }
    
    if ((millis() - lastDebounceTime_) > DEBOUNCE_DELAY_MS) {
        if (reading != stableState_) {
            stableState_ = reading;
            return true; // Состояние изменилось
        }
    }
    
    lastStableState_ = stableState_;
    return false; // Состояние не изменилось
}

ButtonEvent Button::handleStateMachine() {
    static State previousState = STATE_IDLE;
    ButtonEvent event = EVENT_NONE;
    
    // Генерация событий при переходе между состояниями
    if (state_ != previousState) {
        switch (state_) {
            case STATE_RELEASED:
                if (previousState == STATE_PRESSED) {
                    clickCount_++;
                    Logger::debug("Button click count: " + String(clickCount_));
                }
                break;
                
            case STATE_IDLE:
                if (previousState == STATE_RELEASED || previousState == STATE_DOUBLE_WAIT) {
                    if (clickCount_ == 1) {
                        event = EVENT_SINGLE_CLICK;
                        Logger::info("Single click detected");
                    } else if (clickCount_ == 2) {
                        event = EVENT_DOUBLE_CLICK;
                        Logger::info("Double click detected");
                    }
                    clickCount_ = 0;
                }
                break;
                
            case STATE_LONG_PRESS:
                if (previousState == STATE_PRESSED) {
                    event = EVENT_LONG_PRESS;
                    Logger::info("Long press detected");
                    clickCount_ = 0; // Сбрасываем счетчик кликов
                }
                break;
                
            default:
                break;
        }
    }
    
    previousState = state_;
    return event;
}