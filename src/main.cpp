#include <Arduino.h>
#include "button/button.h"
#include "pwm/pwm.h"
#include "uart/uart.h"
#include "led/led.h"
#include "common/config.h"
#include "common/logger.h"

// Глобальные объекты
Button button(BUTTON_PIN);
PWMController pwmController(LED_PWM_PIN);
UARTCommandHandler uartHandler;
LED statusLed(LED_STATUS_PIN);

// Очередь FreeRTOS
QueueHandle_t buttonEventQueue;

// Прототипы задач
void buttonTask(void *parameter);
void pwmTask(void *parameter);
void uartTask(void *parameter);
void statusLedTask(void *parameter);

void setup() {
    // Настройка Serial ПЕРВЫМ делом
    Serial.setRxBufferSize(UART_RX_BUFFER_SIZE);
    Serial.begin(UART_BAUDRATE);
    delay(1000);
    
    Logger::info("=== SYSTEM STARTING ===");
    
    // Инициализация модулей
    button.begin();
    pwmController.begin();
    uartHandler.begin();
    statusLed.begin();
    
    // UART callbacks
    uartHandler.setPWMCallback([](uint8_t dutyCycle) {
        pwmController.setDutyCycle(dutyCycle);
    });
    
    uartHandler.getPWMCallback([]() {
        return pwmController.getDutyCycle();
    });
    
    // Создание очереди
    buttonEventQueue = xQueueCreate(10, sizeof(ButtonEvent));
    
    // Создание задач FreeRTOS
    xTaskCreate(buttonTask, "Button", 4096, NULL, 3, NULL);
    xTaskCreate(pwmTask, "PWM", 4096, NULL, 2, NULL);
    xTaskCreate(uartTask, "UART", 4096, NULL, 2, NULL);
    xTaskCreate(statusLedTask, "StatusLED", 2048, NULL, 1, NULL);
    
    Logger::info("FreeRTOS tasks started");
    Logger::info("Button commands: single=+, double=0, long=cycle");
    Logger::info("UART commands: SET PWM X, GET PWM");
    Logger::info("UART buffer: " + String(UART_RX_BUFFER_SIZE) + " bytes ring buffer");
    
    vTaskDelete(NULL);
}

void loop() {
    vTaskDelete(NULL);
}

// Задача 1: Обработка кнопки - ДИАГНОСТИЧЕСКАЯ ВЕРСИЯ
void buttonTask(void *parameter) {
    ButtonEvent event;
    unsigned long lastStateChangeTime = 0;
    bool lastPhysicalState = HIGH;
    
    while (1) {
        // Прямой мониторинг физического состояния кнопки
        bool currentPhysicalState = digitalRead(BUTTON_PIN);
        
        if (currentPhysicalState != lastPhysicalState) {
            if (currentPhysicalState == LOW) {
                Logger::info("=== PHYSICAL BUTTON PRESSED ===");
            } else {
                Logger::info("=== PHYSICAL BUTTON RELEASED ===");
            }
            lastPhysicalState = currentPhysicalState;
            lastStateChangeTime = millis();
        }
        
        // Обновление логики кнопки
        button.update();
        event = button.getEvent();
        
        if (event != EVENT_NONE) {
            String eventStr;
            switch (event) {
                case EVENT_SINGLE_CLICK: eventStr = "SINGLE_CLICK"; break;
                case EVENT_DOUBLE_CLICK: eventStr = "DOUBLE_CLICK"; break;
                case EVENT_LONG_PRESS: eventStr = "LONG_PRESS"; break;
                default: eventStr = "UNKNOWN"; break;
            }
            Logger::info(">>> BUTTON EVENT: " + eventStr + " <<<");
            
            if (xQueueSend(buttonEventQueue, &event, 0) == pdTRUE) {
                Logger::info("Event sent to PWM task");
            }
        }
        
        // Отладочная информация о состоянии
        static unsigned long lastDebugTime = 0;
        if (millis() - lastDebugTime > 3000) {
            Logger::debug("Button state: " + String(button.isPressed() ? "PRESSED" : "RELEASED") +
                         ", Press time: " + String(millis() - lastStateChangeTime) + "ms");
            lastDebugTime = millis();
        }
        
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

// Задача 2: Управление ШИМ
void pwmTask(void *parameter) {
    ButtonEvent event;
    bool longPressActive = false;
    unsigned long lastLongPressTime = 0;
    
    while (1) {
        if (xQueueReceive(buttonEventQueue, &event, pdMS_TO_TICKS(50))) {
            switch (event) {
                case EVENT_SINGLE_CLICK:
                    pwmController.increaseDutyCycle();
                    Logger::info("SINGLE CLICK - PWM: " + String(pwmController.getDutyCycle()) + "%");
                    longPressActive = false;
                    statusLed.toggle();
                    break;
                    
                case EVENT_DOUBLE_CLICK:
                    pwmController.setDutyCycle(0);
                    Logger::info("DOUBLE CLICK - PWM: 0%");
                    longPressActive = false;
                    statusLed.toggle();
                    break;
                    
                case EVENT_LONG_PRESS:
                    longPressActive = true;
                    lastLongPressTime = millis();
                    Logger::info("LONG PRESS STARTED - cyclic PWM change");
                    // Сразу делаем первое изменение
                    pwmController.handleLongPress();
                    lastLongPressTime = millis();
                    break;
                    
                default:
                    break;
            }
        }
        
        // Обработка активного длительного нажатия
        if (longPressActive && button.isPressed()) {
            if (millis() - lastLongPressTime > PWM_LONG_PRESS_INTERVAL_MS) {
                pwmController.handleLongPress();
                lastLongPressTime = millis();
                Logger::debug("Long press PWM: " + String(pwmController.getDutyCycle()) + "%");
            }
        } else if (longPressActive && !button.isPressed()) {
            // Завершение длительного нажатия
            longPressActive = false;
            pwmController.resetLongPressCycle();
            Logger::info("LONG PRESS ENDED");
        }
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// Задача 3: Обработка UART
void uartTask(void *parameter) {
    while (1) {
        uartHandler.processCommands();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Задача 4: Мигание статусным LED
void statusLedTask(void *parameter) {
    while (1) {
        statusLed.toggle();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}