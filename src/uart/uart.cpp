#include "uart.h"

// ============================================================================
// Реализация кольцевого буфера
// ============================================================================

UARTCommandHandler::RingBuffer::RingBuffer() 
    : head_(0), tail_(0), count_(0) {
    memset(data_, 0, sizeof(data_));
}

bool UARTCommandHandler::RingBuffer::put(char c) {
    if (isFull()) {
        return false; // Буфер переполнен
    }
    
    data_[head_] = c;
    head_ = (head_ + 1) % UART_RX_BUFFER_SIZE;
    count_++;
    return true;
}

bool UARTCommandHandler::RingBuffer::get(char* c) {
    if (isEmpty()) {
        return false; // Буфер пуст
    }
    
    *c = data_[tail_];
    tail_ = (tail_ + 1) % UART_RX_BUFFER_SIZE;
    count_--;
    return true;
}

void UARTCommandHandler::RingBuffer::clear() {
    head_ = 0;
    tail_ = 0;
    count_ = 0;
    memset(data_, 0, sizeof(data_));
}

bool UARTCommandHandler::RingBuffer::isFull() const {
    return count_ >= UART_RX_BUFFER_SIZE;
}

bool UARTCommandHandler::RingBuffer::isEmpty() const {
    return count_ == 0;
}

uint16_t UARTCommandHandler::RingBuffer::available() const {
    return count_;
}

// ============================================================================
// Реализация обработчика UART команд
// ============================================================================

UARTCommandHandler::UARTCommandHandler() 
    : cmdIndex_(0), setPWMCallback_(nullptr), getPWMCallback_(nullptr) {
    memset(cmdBuffer_, 0, sizeof(cmdBuffer_));
}

void UARTCommandHandler::begin() {
    // Установка размера буфера ДО начала Serial
    Serial.setRxBufferSize(UART_RX_BUFFER_SIZE);
    Serial.begin(UART_BAUDRATE);
    Logger::info("UART initialized with ring buffer " + String(UART_RX_BUFFER_SIZE) + " bytes");
}

void UARTCommandHandler::processCommands() {
    // Шаг 1: Чтение данных из UART в кольцевой буфер
    while (Serial.available() > 0) {
        char c = Serial.read();
        
        if (!rxRingBuffer_.put(c)) {
            // ПЕРЕПОЛНЕНИЕ КОЛЬЦЕВОГО БУФЕРА - критическая ошибка
            handleBufferOverflow();
            break; // Прерываем чтение чтобы стабилизировать систему
        }
    }
    
    // Шаг 2: Обработка данных из кольцевого буфера
    char c;
    while (rxRingBuffer_.get(&c)) {
        if (c == '\n' || c == '\r') {
            // Конец команды
            if (cmdIndex_ > 0) {
                cmdBuffer_[cmdIndex_] = '\0';
                
                // Проверка на переполнение буфера команды
                if (cmdIndex_ >= (UART_CMD_BUFFER_SIZE - 1)) {
                    handleCommandOverflow();
                } else {
                    processCommand(String(cmdBuffer_));
                }
                
                cmdIndex_ = 0;
                memset(cmdBuffer_, 0, sizeof(cmdBuffer_));
            }
        } else if (cmdIndex_ < (UART_CMD_BUFFER_SIZE - 1)) {
            // Накопление символов команды
            cmdBuffer_[cmdIndex_++] = c;
        } else {
            // ПЕРЕПОЛНЕНИЕ БУФЕРА КОМАНДЫ
            handleCommandOverflow();
            
            // Пропускаем оставшиеся символы до конца строки
            char temp;
            while (rxRingBuffer_.get(&temp)) {
                if (temp == '\n' || temp == '\r') {
                    break;
                }
            }
        }
    }
}

void UARTCommandHandler::handleBufferOverflow() {
    /**
     * СТРАТЕГИЯ ОБРАБОТКИ ПЕРЕПОЛНЕНИЯ КОЛЬЦЕВОГО БУФЕРА:
     * 1. Очищаем буфер для освобождения места
     * 2. Уведомляем пользователя об ошибке
     * 3. Логируем инцидент для отладки
     * 4. Продолжаем работу в штатном режиме
     */
    Logger::error("UART ring buffer overflow detected! Clearing buffer.");
    
    // Очистка кольцевого буфера
    rxRingBuffer_.clear();
    
    // Очистка буфера команды
    cmdIndex_ = 0;
    memset(cmdBuffer_, 0, sizeof(cmdBuffer_));
    
    // Уведомление пользователя
    sendResponse("ERROR: Buffer overflow - data lost. Please resend command.");
}

void UARTCommandHandler::handleCommandOverflow() {
    /**
     * СТРАТЕГИЯ ОБРАБОТКИ ПЕРЕПОЛНЕНИЯ БУФЕРА КОМАНДЫ:
     * 1. Сбрасываем текущую команду
     * 2. Уведомляем пользователя
     * 3. Логируем инцидент
     * 4. Продолжаем обработку новых команд
     */
    Logger::error("UART command buffer overflow! Command too long.");
    
    // Сброс текущей команды
    cmdIndex_ = 0;
    memset(cmdBuffer_, 0, sizeof(cmdBuffer_));
    
    // Уведомление пользователя
    sendResponse("ERROR: Command too long - maximum " + String(UART_CMD_BUFFER_SIZE - 1) + " characters allowed");
}

void UARTCommandHandler::setPWMCallback(void (*callback)(uint8_t)) {
    setPWMCallback_ = callback;
}

void UARTCommandHandler::getPWMCallback(uint8_t (*callback)()) {
    getPWMCallback_ = callback;
}

void UARTCommandHandler::processCommand(const String& command) {
    Logger::debug("UART command: " + command);
    
    String cmd = command;
    cmd.toUpperCase();
    cmd.trim();
    
    if (cmd.startsWith("SET PWM")) {
        handleSetPWM(cmd.substring(7));
    } else if (cmd == "GET PWM") {
        handleGetPWM();
    } else if (cmd.length() > 0) {
        sendResponse("ERROR: Unknown command");
        Logger::error("Unknown UART command: " + command);
    }
}

void UARTCommandHandler::sendResponse(const String& response) {
    Serial.println(response);
}

void UARTCommandHandler::handleSetPWM(const String& parameters) {
    if (!setPWMCallback_) {
        sendResponse("ERROR: PWM callback not set");
        return;
    }
    
    String params = parameters;
    params.trim();
    
    // Проверка на пустые параметры
    if (params.length() == 0) {
        sendResponse("ERROR: Missing PWM value. Usage: SET PWM X (0-100)");
        return;
    }
    
    int pwmValue;
    if (validateNumber(params, pwmValue)) {
        if (pwmValue >= 0 && pwmValue <= 100) {
            setPWMCallback_(pwmValue);
            sendResponse("OK");
            Logger::info("UART: PWM set to " + String(pwmValue) + "%");
        } else {
            sendResponse("ERROR: PWM value must be 0-100");
            Logger::error("UART: Invalid PWM value " + String(pwmValue));
        }
    } else {
        sendResponse("ERROR: Invalid number format");
        Logger::error("UART: Invalid number format: " + params);
    }
}

void UARTCommandHandler::handleGetPWM() {
    if (!getPWMCallback_) {
        sendResponse("ERROR: PWM callback not set");
        return;
    }
    
    uint8_t pwmValue = getPWMCallback_();
    sendResponse(String(pwmValue));
    Logger::debug("UART: GET PWM returned " + String(pwmValue));
}

bool UARTCommandHandler::validateNumber(const String& str, int& value) {
    String numStr = str;
    numStr.trim();
    
    // Проверка на пустую строку
    if (numStr.length() == 0) {
        return false;
    }
    
    // Проверка что все символы - цифры
    for (size_t i = 0; i < numStr.length(); i++) {
        if (!isdigit(numStr[i])) {
            return false;
        }
    }
    
    value = numStr.toInt();
    return true;
}