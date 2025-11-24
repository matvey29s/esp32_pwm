#include "uart.h"

UARTCommandHandler::UARTCommandHandler() : rxIndex_(0), setPWMCallback_(nullptr), 
                                          getPWMCallback_(nullptr) {
    memset(rxBuffer_, 0, sizeof(rxBuffer_));
}

void UARTCommandHandler::processCommands() {
    while (Serial.available() > 0) {
        char c = Serial.read();
        
        if (c == '\n' || c == '\r') {
            if (rxIndex_ > 0) {
                rxBuffer_[rxIndex_] = '\0';
                processCommand(String(rxBuffer_));
                rxIndex_ = 0;
            }
        } else if (rxIndex_ < (UART_RX_BUFFER_SIZE - 1)) {
            rxBuffer_[rxIndex_++] = c;
        } else {
            Logger::error("UART buffer overflow - resetting buffer");
            rxIndex_ = 0;
            memset(rxBuffer_, 0, sizeof(rxBuffer_));
            sendResponse("ERROR: Buffer overflow");
        }
    }
}

void UARTCommandHandler::setPWMCallback(void (*callback)(uint8_t)) {
    setPWMCallback_ = callback;
}

void UARTCommandHandler::getPWMCallback(uint8_t (*callback)()) {
    getPWMCallback_ = callback;
}

void UARTCommandHandler::processCommand(const String& command) {
    Logger::debug("Received command: " + command);
    
    String cmd = command;
    cmd.toUpperCase();
    cmd.trim();
    
    if (cmd.startsWith("SET PWM")) {
        handleSetPWM(cmd.substring(7));
    } else if (cmd == "GET PWM") {
        handleGetPWM();
    } else {
        sendResponse("ERROR: Unknown command");
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
    
    int pwmValue;
    if (validateNumber(params, pwmValue)) {
        if (pwmValue >= 0 && pwmValue <= 100) {
            setPWMCallback_(pwmValue);
            sendResponse("OK");
        } else {
            sendResponse("ERROR: PWM value must be 0-100");
        }
    } else {
        sendResponse("ERROR: Invalid number format");
    }
}

void UARTCommandHandler::handleGetPWM() {
    if (!getPWMCallback_) {
        sendResponse("ERROR: PWM callback not set");
        return;
    }
    
    uint8_t pwmValue = getPWMCallback_();
    sendResponse(String(pwmValue));
}

bool UARTCommandHandler::validateNumber(const String& str, int& value) {
    String numStr = str;
    numStr.trim();
    
    for (size_t i = 0; i < numStr.length(); i++) {
        if (!isdigit(numStr[i])) {
            return false;
        }
    }
    
    value = numStr.toInt();
    return true;
}