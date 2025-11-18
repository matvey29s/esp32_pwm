#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

class Logger {
public:
    static void info(const String& message) {
        Serial.print("[INFO] ");
        Serial.println(message);
    }
    
    static void error(const String& message) {
        Serial.print("[ERROR] ");
        Serial.println(message);
    }
    
    static void debug(const String& message) {
        #ifdef DEBUG
        Serial.print("[DEBUG] ");
        Serial.println(message);
        #endif
    }
};

#endif