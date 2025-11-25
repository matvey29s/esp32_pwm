#ifndef CONFIG_H
#define CONFIG_H

// Конфигурация пинов ESP32
#define BUTTON_PIN 4
#define LED_PWM_PIN 2  
#define LED_STATUS_PIN 13

// Конфигурация UART
#define UART_BAUDRATE 115200
#define UART_RX_BUFFER_SIZE 256  // Размер кольцевого буфера
#define UART_CMD_BUFFER_SIZE 128 // Размер буфера для команд

// Конфигурация кнопки (увеличим времена для надежности)
#define DEBOUNCE_DELAY_MS 50
#define DOUBLE_CLICK_MAX_MS 600    // 600ms для двойного клика
#define LONG_PRESS_MIN_MS 1200     // 1.2 секунды для длительного нажатия

// Конфигурация ШИМ
#define PWM_MIN 0
#define PWM_MAX 100
#define PWM_STEP 10
#define PWM_LONG_PRESS_INTERVAL_MS 1000

#endif