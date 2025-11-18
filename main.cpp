#include <Arduino.h>
#include "button/button.h"
#include "pwm/pwm.h"
#include "uart/uart.h"
#include "led/led.h"
#include "common/config.h"
#include "common/logger.h"

// Глобальные объекты
Button button(BUTTON_PIN);
PWMController pwmController(LED_PIN);
UARTCommandHandler uartHandler;
LED statusLed(LED_BUILTIN);

// Переменные для управления длительным нажатием
bool longPressActive = false;
unsigned long lastLongPressUpdate = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Logger::info("System starting...");
    
    // Инициализация модулей
    button.begin();
    pwmController.begin();
    uartHandler.begin();
    statusLed.begin();
    
    // Настройка callback'ов для UART
    uartHandler.setPWMCallback([](uint8_t dutyCycle) {
        pwmController.setDutyCycle(dutyCycle);
    });
    
    uartHandler.getPWMCallback([]() {
        return pwmController.getDutyCycle();
    });
    
    Logger::info("System started successfully");
}

void loop() {
    static unsigned long lastBlinkTime = 0;
    
    // Задача 1: Обработка кнопки
    button.update();
    ButtonEvent event = button.getEvent();
    
    switch (event) {
        case EVENT_SINGLE_CLICK:
            // Однократное нажатие - увеличиваем ШИМ
            pwmController.increaseDutyCycle();
            statusLed.toggle();
            longPressActive = false;
            Logger::debug("Single click - PWM increased");
            break;
            
        case EVENT_DOUBLE_CLICK:
            // Двойное нажатие - сбрасываем ШИМ в 0
            pwmController.setDutyCycle(0);
            statusLed.toggle();
            longPressActive = false;
            Logger::debug("Double click - PWM reset to 0");
            break;
            
        case EVENT_LONG_PRESS:
            // Длительное нажатие - начинаем циклическое изменение
            longPressActive = true;
            lastLongPressUpdate = millis();
            Logger::debug("Long press started - cyclic PWM change");
            break;
            
        case EVENT_NONE:
            // Обработка активного длительного нажатия
            if (longPressActive && button.isPressed()) {
                if (millis() - lastLongPressUpdate >= PWM_LONG_PRESS_INTERVAL_MS) {
                    pwmController.handleLongPress();
                    lastLongPressUpdate = millis();
                    Logger::debug("Long press active - PWM: " + String(pwmController.getDutyCycle()) + "%");
                }
            } else if (longPressActive && !button.isPressed()) {
                // Завершение длительного нажатия
                longPressActive = false;
                pwmController.resetLongPressCycle();
                Logger::debug("Long press ended");
            }
            break;
    }
    
    // Задача 2: Обработка UART команд
    uartHandler.processCommands();
    
    // Задача 3: Мигающий LED для индикации работы
    if (millis() - lastBlinkTime > 1000) {
        statusLed.toggle();
        lastBlinkTime = millis();
    }
    
    delay(10); // Небольшая задержка для стабильности
}