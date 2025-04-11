#include "ble_handlers.h"
#include "ble_controller.h"
extern "C"
{
#define LUAT_LOG_TAG "BLE_HANDLERS"
#include "luat_log.h"
}
#define CHECK_ABORT()                                           \
    if (bleController.getIsBlockingOperationRunning() == false) \
    {                                                           \
        TEST_PRINTLN("Operation aborted");                      \
        break;                                                  \
    }
    void onBLEConnect()
    {
        // reset the shutdown timer
        LLOGI("BLE Connected!");
    
        // buzzer.beep(800, 300, 1);
    }
void initializeBLEHandlers()
{
    

    // ble callback to get hardware version

    bleController.registerMessageCallback("print", handlePrintMessage);

    bleController.registerMessageCallback("settextmode", handleTextMode);
    bleController.registerMessageCallback("lua", handleTextMode);

}

void onBLEDisconnect()
{
    TEST_PRINTLN("BLE Disconnected!");
    // restart the shutdown timer
    // restar bluetooth advertisement

    // buzzer.stop();

    // bleController.switchToJsonMode(); for forced lua mode
    // buzzer.beep(400, 300, 2);
}

void onOtaStart()
{
    TEST_PRINTLN("OTA update started");
    // buzzer.beepAsync(400, 0, 1);
    JsonDocument responce;
    responce["msgtyp"] = "otastart";
    bleController.sendMessage(responce);
}
void onOtaProgress(int percentage)
{
    Serial.printf("OTA progress: %d%%\n", percentage);
}

void onOtaSuccess()
{
    TEST_PRINTLN("OTA update successful, restarting...");
    // buzzer.beepAsync(1500, 0, 1);
    JsonDocument responce;
    responce["msgtyp"] = "ota";
    responce["result"] = 1;
    bleController.sendMessage(responce);
}

void onOtaError(const char *error)
{
    Serial.printf("OTA error: %s\n", error);
    // buzzer.beepAsync(500, 200, 3);
    JsonDocument responce;
    responce["msgtyp"] = "ota";
    responce["result"] = 0;
    responce["error"] = error;
    bleController.sendMessage(responce);
}





void processBLEMessages()
{
    bleController.processJsonMessages();
    bleController.processTextMessages();
}

/***************************************PERAMETERS STORAGE********************************************** */
/********************************************DEVICE CONFIG***********************************************/

void handleGetDeviceName(JsonDocument &doc)
{

    String name = "madarchod";
    JsonDocument response;
    response["msgtyp"] = "getname";
    response["name"] = name;
    bleController.sendMessage(response);
}

void handleSetDeviceName(JsonDocument &doc)
{
    String name = doc["newname"];
    bleController.setDeviceName(name);
}

/********************************************DEVICE TESTS***********************************************/

void handlePrintMessage(JsonDocument &doc)
{
    int readTime = 10000;
    int16_t distance, flux;
    if (doc.containsKey("time"))
    {
        readTime = doc["time"];
    }

    // read tf sensor and send at 50ms until readTime and send to ble
    JsonDocument response;
    response["msgtyp"] = "print";

    // tfSensor.start();

    long start = millis();

    while (millis() - start < readTime)
    {
        // tfSensor.readDisFlux(distance, flux);
        response["distance"] = 6;
        response["flux"] = 12;
        bleController.sendMessage(response);
        delay(50);
    }

    // tfSensor.stop();
}

void handleTextMode(JsonDocument &doc)
{
    bleController.switchToTextMode();
}