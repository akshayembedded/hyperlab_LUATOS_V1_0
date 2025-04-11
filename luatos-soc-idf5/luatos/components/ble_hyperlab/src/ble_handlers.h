#ifndef BLE_HANDLERS_H
#define BLE_HANDLERS_H

#include <ArduinoJson.h>



void initializeBLEHandlers();


void handlePrintMessage(JsonDocument &doc);


// void handleDrillMessage(JsonDocument &doc);



void handleTextMode(JsonDocument &doc);
// void handleStartTimer(JsonDocument &doc);
// void drillTask(void *pvParameters);

void processBLEMessages();
void onBLEConnect();
void onBLEDisconnect();
void onOtaStart();
void onOtaProgress(int percentage);
void onOtaSuccess();
void onOtaError(const char *error);

// Add more handler declarations as needed

#endif // BLE_HANDLERS_H
// // #include "motor/motion_system.h"
// // #include "global.h"
// // #include "drill_handler.h"

// void initializeBLEHandlers();

// void handleMoveMessage(JsonDocument &msg);
// // void handleHomeMessage(JsonDocument &doc);

// void handleCalibrateMessage(JsonDocument &doc);
// void handleCalibrateWithDistanceMessage(JsonDocument &doc);
// void handleSetHomeDistance(JsonDocument &doc);
// void handleSetYOffset(JsonDocument &doc);

// void handleGetVersion(JsonDocument &doc);
// void handleHWVersion(JsonDocument &doc);
// void handleGetBleMac(JsonDocument &doc);
// void handleAbortMessage(JsonDocument &doc);
// void getJsonPeramter(JsonDocument &doc);
// void storeJsonPeramter(JsonDocument &doc);
// void clearJsonPeramter(JsonDocument &doc);
// void handleLaserMessage(JsonDocument &doc);
// void handlePowerMessage(JsonDocument &doc);
// void handleResetOffset(JsonDocument &doc);
// void handleErrorCalculation(JsonDocument &doc);
// void handleApplyOffset(JsonDocument &doc);
// void handleRestoreCalibration(JsonDocument &doc);
// void handleMotorXwiggleMultiplier(JsonDocument &doc);
// void handleAutoCalibrate(JsonDocument &doc);
// void handleCalibrateBox(JsonDocument &doc);
// void handlePrintMessage(JsonDocument &doc);

// void handleGetDeviceName(JsonDocument &doc);
// void handleSetDeviceName(JsonDocument &doc);
// void handleAutoShutdown(JsonDocument &doc);
// // void handleDrillMessage(JsonDocument &doc);

// void handleFluxThresold(JsonDocument &doc);
// void handleDistanceTolerance(JsonDocument &doc);
// void handleParameters(JsonDocument &doc);


// void handleTextMode(JsonDocument &doc);
// // void handleStartTimer(JsonDocument &doc);
// // void drillTask(void *pvParameters);

// void processBLEMessages();
// void onBLEConnect();
// void onBLEDisconnect();
// void onOtaStart();
// void onOtaProgress(int percentage);
// void onOtaSuccess();
// void onOtaError(const char *error);

// // Add more handler declarations as needed

// #endif // BLE_HANDLERS_H