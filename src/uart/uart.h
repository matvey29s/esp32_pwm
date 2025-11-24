#ifndef UART_H
#define UART_H

#include <Arduino.h>
#include "../common/config.h"
#include "../common/logger.h"

class UARTCommandHandler {
public:
    UARTCommandHandler();
    void processCommands();
    void setPWMCallback(void (*callback)(uint8_t));
    void getPWMCallback(uint8_t (*callback)());

private:
    char rxBuffer_[UART_RX_BUFFER_SIZE];
    uint16_t rxIndex_;
    void (*setPWMCallback_)(uint8_t);
    uint8_t (*getPWMCallback_)();
    
    void processCommand(const String& command);
    void sendResponse(const String& response);
    void handleSetPWM(const String& parameters);
    void handleGetPWM();
    bool validateNumber(const String& str, int& value);
};

#endif