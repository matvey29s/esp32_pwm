#ifndef UART_H
#define UART_H

#include <Arduino.h>
#include "../common/config.h"
#include "../common/logger.h"

class UARTCommandHandler {
public:
    UARTCommandHandler();
    void begin();
    void processCommands();
    void setPWMCallback(void (*callback)(uint8_t));
    void getPWMCallback(uint8_t (*callback)());

private:
    // Кольцевой буфер для приема данных
    class RingBuffer {
    private:
        char data_[UART_RX_BUFFER_SIZE];
        uint16_t head_;
        uint16_t tail_;
        uint16_t count_;
        
    public:
        RingBuffer();
        bool put(char c);
        bool get(char* c);
        void clear();
        bool isFull() const;
        bool isEmpty() const;
        uint16_t available() const;
    };
    
    RingBuffer rxRingBuffer_;           // Кольцевой буфер 256 байт
    char cmdBuffer_[UART_CMD_BUFFER_SIZE]; // Буфер для сборки команд
    uint16_t cmdIndex_;
    void (*setPWMCallback_)(uint8_t);
    uint8_t (*getPWMCallback_)();
    
    void processCommand(const String& command);
    void sendResponse(const String& response);
    void handleSetPWM(const String& parameters);
    void handleGetPWM();
    bool validateNumber(const String& str, int& value);
    void handleBufferOverflow();
    void handleCommandOverflow();
};

#endif