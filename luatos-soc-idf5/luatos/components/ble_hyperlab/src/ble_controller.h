#ifndef BLE_CONTROLLER_H
#define BLE_CONTROLLER_H

#define ARDUINOJSON_ENABLE_STD_STRING 1
#include <Arduino.h>
#include <NimBLEDevice.h>
#include <ArduinoJson.h>
#include <queue>
#include <string>
#include <functional>
#include <map>
#include "esp_ota_ops.h"
#include "config.h"

#include "SPIFFS.h"
#include "FFat.h"
#include "spiffsfile.h"
#include <Update.h>

#define OTA_SERVICE_UUID "fb1e4001-54ae-4a28-9f74-dfccb248601d"
#define OTA_UUID_RX "fb1e4002-54ae-4a28-9f74-dfccb248601d"
#define OTA_UUID_TX "fb1e4003-54ae-4a28-9f74-dfccb248601d"
#define PRODUCT_UUID "AE09"
void initializeBLEController(String );
class BLEController
{
public:
    BLEController();
    void begin(const String &deviceName);
    void setDeviceName(String deviceName);

    void processJsonMessages();
    void processTextMessages();

    void updateBatteryAndVersion(int batteryLevel, const uint8_t *SWversion, size_t SWversionLength, uint8_t *HWVersion, size_t HWVersionLength);
    void setServiceUUID(const char *uuid);
    void setCharacteristicUUIDs(const char *txUuid, const char *rxUuid);
    void setOnConnectCallback(std::function<void()> callback);
    void setOnDisconnectCallback(std::function<void()> callback);
    void setOtaCallbacks(std::function<void()> startCallback,
                         std::function<void(int)> progressCallback,
                         std::function<void()> successCallback,
                         std::function<void(const char *)> errorCallback);

    void sendMessage(const JsonDocument &message);
    bool transferFileFromSPIFFS(const char *filename, uint16_t mtu);
    void sendMessage(String message);
    void registerMessageCallback(const String &msgType, std::function<void(JsonDocument &)> callback);
    void registerHighPriorityCallback(const String &msgType, std::function<void(JsonDocument &)> callback);

    void setIsBlockingOperationRunning(bool isRunning);
    bool getIsBlockingOperationRunning();
    void clearMessageQueue();
    bool isBlockingOperationRunning;
    void updateBattery(int batteryLevel);

    void stop();

    String getMacAddress();

    void sendTextOutput(const std::string &output);
    void switchToTextMode();
    void switchToJsonMode();
    void setTextMessageCallback(std::function<void(String)> callback)
    {
        TextMessageCallback = callback;
    }

    void setTextAbortCallback(std::function<void(String)> callback)
    {
        TextAbortCallback = callback;
    }
        void setTextQueueCallback(std::function<void(String)> callback)
    {
        TextQueueCallback = callback;
    }
    void handleOTA(String data, uint8_t msgId);
    void receiveOTA(const std::string &pData);
    void processOTAData();
    void sendResponseBinaryOTA(byte data, uint8_t msgId);
    void sendResponseJsonOTA(String data, String msgtype, uint8_t msgId);
    void sendmsgOTA(String str);

    struct FileTransferStatus
    {
        String currentFileName;
        int fileIndex;
        size_t totalSize;
        size_t bytesTransferred;
        int progressPercentage;
    };

    FileTransferStatus getFileTransferStatus();
    void handleFileTransfer(JsonDocument &doc);
    bool updateFromFS(fs::FS &fs, const char *path = "/update.bin");

    bool deviceConnected;
    int mtu;
    String serviceUUID;
    String txUUID;
    String rxUUID;
    String batUUID;
    String productUUID;
    String otaServiceUUID;
    String otaTxUUID;
    String otaRxUUID;
    String deviceName;
    String macAddress;

    // esp_ota_handle_t otaHandler = 0;

    std::queue<std::string> msgota;
    std::string blankota;

private:
    bool _initialized;
    struct Message
    {
        String type;
        JsonDocument doc;
        Message(const String &t, const JsonDocument &d) : type(t), doc(d) {}
    };

    BLEServer *pServer;
    BLECharacteristic *pTxCharacteristic;
    BLECharacteristic *pRxCharacteristic;
    BLECharacteristic *pBatCharacteristic;
    BLECharacteristic *otaTX;
    BLECharacteristic *otaRX;
    //     static BLECharacteristic *otaTX;
    // static BLECharacteristic *otaRX;
    std::queue<Message> messageQueue;
    std::map<String, std::function<void(JsonDocument &)>> messageCallbacks;
    std::map<String, std::function<void(JsonDocument &)>> highPriorityCallbacks;

    std::function<void()> onConnectCallback;
    std::function<void()> onDisconnectCallback;
    std::function<void()> otaStartCallback;
    std::function<void(int)> otaProgressCallback;
    std::function<void()> otaSuccessCallback;
    std::function<void(const char *)> otaErrorCallback;
    std::function<void(String)> TextMessageCallback;
    std::function<void(String)> TextAbortCallback;
    std::function<void(String)> TextQueueCallback;


    void setManufacturerData(BLEAdvertisementData &adData);
    void startAdvertising();
    void initOTA();
    void handleReceivedMessage(const std::string &message);
    void processHighPriorityMessage(Message &msg);
    void handleMtuChange(uint16_t newMtu);

    class ServerCallbacks;
    class CharCallbacks;
    class OtaCallbacks;

    int currentBatteryLevel;
    uint8_t currentSWVersion[3];
    uint8_t currentHWVersion[3];
    esp_ota_handle_t otaHandler;
    std::string receivedData;

    bool TextMode;
    std::string TextBuffer;
    std::queue<std::string> TextOutputQueue;
    bool textBufferIsExecuting;
    bool executeTextBufferFlag;
    void executeTextBuffer();

    // File transfer tracking
    FileTransferStatus fileStatus;
    void updateFileTransferProgress(size_t bytesReceived);
    void resetFileTransferStatus();

    // File transfer helpers
    bool performUpdate(Stream &updateSource, size_t updateSize);
    void handleFileOpen(JsonDocument &doc);
    void handleFileWrite(JsonDocument &doc);
    void handleFileClose(JsonDocument &doc);
};

extern BLEController bleController;

#endif // BLE_CONTROLLER_H