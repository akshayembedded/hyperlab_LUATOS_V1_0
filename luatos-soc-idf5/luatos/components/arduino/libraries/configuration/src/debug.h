#ifndef ESP32_DEBUG_H
#define ESP32_DEBUG_H

#include <Arduino.h>
#include <esp_task_wdt.h>
// Comment out this line to disable all debug output
//  #define DEBUG_ENABLED

// Define log levels as bit flags
#define LOG_LEVEL_INFO    (1 << 0)
#define LOG_LEVEL_ERROR   (1 << 1)
#define LOG_LEVEL_TESTING (1 << 2)

// Set the current log levels here (combine with bitwise OR |)
#define CURRENT_LOG_LEVELS (  LOG_LEVEL_ERROR | LOG_LEVEL_TESTING | LOG_LEVEL_INFO)
// #define CURRENT_LOG_LEVELS (  LOG_LEVEL_ERROR | LOG_LEVEL_TESTING )


#ifdef DEBUG_ENABLED
    #define DEBUG_PRINT(level, fmt, ...) \
        do { \
            if (CURRENT_LOG_LEVELS & level) { \
                Serial.printf("[%s][%lu] ", \
                    level == LOG_LEVEL_INFO ? "INFO" : \
                    level == LOG_LEVEL_ERROR ? "ERROR" : \
                    "TEST", \
                    millis()); \
                Serial.printf(fmt, ##__VA_ARGS__); \
            } \
        } while (0)

    #define DEBUG_PRINTLN(level, fmt, ...) \
        do { \
            if (CURRENT_LOG_LEVELS & level) { \
                Serial.printf("[%s][%lu] ", \
                    level == LOG_LEVEL_INFO ? "INFO" : \
                    level == LOG_LEVEL_ERROR ? "ERROR" : \
                    "TEST", \
                    millis()); \
                Serial.printf(fmt, ##__VA_ARGS__); \
                Serial.println(); \
            } \
        } while (0)
#else
    #define DEBUG_PRINT(level, fmt, ...)
    #define DEBUG_PRINTLN(level, fmt, ...)
#endif

// Convenience macros for different log levels
#define INFO_PRINT(fmt, ...)    DEBUG_PRINT(LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define INFO_PRINTLN(fmt, ...)  DEBUG_PRINTLN(LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define ERROR_PRINT(fmt, ...)   DEBUG_PRINT(LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define ERROR_PRINTLN(fmt, ...) DEBUG_PRINTLN(LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define TEST_PRINT(fmt, ...)    DEBUG_PRINT(LOG_LEVEL_TESTING, fmt, ##__VA_ARGS__)
#define TEST_PRINTLN(fmt, ...)  DEBUG_PRINTLN(LOG_LEVEL_TESTING, fmt, ##__VA_ARGS__)

#endif // ESP32_DEBUG_H