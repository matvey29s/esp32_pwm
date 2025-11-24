#ifndef CONFIG_H
#define CONFIG_H

// Конфигурация пинов ESP32
#define BUTTON_PIN 4       // GPIO0 для кнопки
#define LED_PWM_PIN 2       // GPIO2 для ШИМ светодиода  
#define LED_STATUS_PIN 13   // GPIO13 для статусного светодиода

// Конфигурация UART
#define UART_BAUDRATE 115200
#define UART_RX_BUFFER_SIZE 256

// Конфигурация кнопки
#define DEBOUNCE_DELAY_MS 50        // Время фильтрации дребезга
#define DOUBLE_CLICK_MAX_MS 500     // Макс время между кликами для двойного нажатия
#define LONG_PRESS_MIN_MS 1000      // Мин время для длительного нажатия

// Конфигурация ШИМ
#define PWM_MIN 0           // Минимальная скважность %
#define PWM_MAX 100         // Максимальная скважность %
#define PWM_STEP 10         // Шаг изменения скважности
#define PWM_LONG_PRESS_INTERVAL_MS 1000  // Интервал изменения при длительном нажатии

#endif