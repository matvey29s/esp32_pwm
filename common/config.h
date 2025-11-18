#ifndef CONFIG_H
#define CONFIG_H

// Конфигурация пинов
#define BUTTON_PIN 0
#define LED_PIN 2

// Конфигурация UART
#define UART_BAUDRATE 115200
#define UART_RX_BUFFER_SIZE 256
#define UART_TX_BUFFER_SIZE 256

// Конфигурация кнопки
#define DEBOUNCE_DELAY_MS 50
#define DOUBLE_CLICK_MAX_MS 400
#define LONG_PRESS_MIN_MS 1000

// Конфигурация ШИМ
#define PWM_MIN 0
#define PWM_MAX 100
#define PWM_STEP 10
#define PWM_LONG_PRESS_INTERVAL_MS 2000

#endif