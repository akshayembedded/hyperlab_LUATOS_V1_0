
#include "ble_controller.h"
#include "ble_handlers.h"
#include <Arduino.h>
extern "C"
{
#define LUAT_LOG_TAG "BLE_CTRL"
#include "luat_log.h"
}
BLEController bleController;
extern void lua_loop(String script);
extern void luaClose(String script);

class BLEController::ServerCallbacks : public BLEServerCallbacks {
    BLEController &controller;

public:
    ServerCallbacks(BLEController &ctrl) : controller(ctrl) {}

    void onConnect(BLEServer *pServer, NimBLEConnInfo& connInfo) override {
        {
        LLOGI("Trying to connect");
        controller.deviceConnected = true;
        if (controller.onConnectCallback) {
            controller.onConnectCallback();
        }
        // vTaskDelay(1 / portTICK_PERIOD_MS);
        pServer->updateConnParams(connInfo.getConnHandle(), 24, 48, 0, 180);
    }
}

    void onDisconnect(BLEServer *pServer, NimBLEConnInfo& connInfo, int reason) override{
         
        controller.deviceConnected = false;
        if (controller.onDisconnectCallback) {
            controller.onDisconnectCallback();
        }
        controller.startAdvertising();
    }

        

    void onMTUChange( uint16_t MTU, NimBLEConnInfo& connInfo) override {
        LLOGI("MTU updated: %u for connection ID: %u\n", MTU, connInfo.getConnHandle());
        
    }
};

class BLEController::CharCallbacks : public BLECharacteristicCallbacks {
    BLEController &controller;

public:
    CharCallbacks(BLEController &ctrl) : controller(ctrl) {}

    void onWrite(BLECharacteristic *pCharacteristic, NimBLEConnInfo& connInfo) override  {
        std::string rxValue = pCharacteristic->getValue();
        controller.handleReceivedMessage(rxValue);
        rxValue.clear();
    }
};

class BLEController::OtaCallbacks : public BLECharacteristicCallbacks {
    BLEController &controller;

public:
    OtaCallbacks(BLEController &ctrl) : controller(ctrl) {}

    void onWrite(BLECharacteristic *pCharacteristic, NimBLEConnInfo& connInfo) override  {
        std::string pData = pCharacteristic->getValue();
        int len = pData.length();
        controller.receiveOTA(pData);
    }
};


void BLEController::receiveOTA(const std::string &pData)
{
    if (pData.length() > 0)
    {
        if (pData.find('\4') != std::string::npos)
        {
            blankota = blankota + pData;
            blankota.erase(blankota.length() - 1);
            msgota.push(blankota);
            blankota = "";
        }
        else
        {
            blankota = blankota + pData;
        }
    }
}


BLEController::BLEController() : deviceConnected(false), mtu(250),
                                 serviceUUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E"),
                                 txUUID("6E400003-B5A3-F393-E0A9-E50E24DCCA9E"),
                                 rxUUID("6E400002-B5A3-F393-E0A9-E50E24DCCA9E"),
                                 batUUID("180F"),
                                 productUUID(PRODUCT_UUID),
                                 otaServiceUUID("fb1e4001-54ae-4a28-9f74-dfccb248601d"),
                                 otaTxUUID("fb1e4003-54ae-4a28-9f74-dfccb248601d"),
                                 otaRxUUID("fb1e4002-54ae-4a28-9f74-dfccb248601d"),
                                 currentBatteryLevel(0), otaHandler(0),
                                 TextMode(false)
{
    memset(currentSWVersion, 0, sizeof(currentSWVersion));
}


void BLEController::begin(const String &Name)
{
    // INFO_PRINTLN(" BLEController::begin()");
    this->deviceName = Name;

    // INFO_PRINTLN("Starting BLE Device: %s", this->deviceName.c_str());
    BLEDevice::init(this->deviceName.c_str());
    BLEDevice::setPower(ESP_PWR_LVL_P9);
    BLEDevice::setMTU(512);

    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks(*this));

    BLEService *pService = pServer->createService(serviceUUID.c_str());
    BLEService *pBatService = pServer->createService(batUUID.c_str());

    pTxCharacteristic = pService->createCharacteristic(txUUID.c_str(), NIMBLE_PROPERTY::NOTIFY);
    pRxCharacteristic = pService->createCharacteristic(rxUUID.c_str(), NIMBLE_PROPERTY::WRITE);
    pBatCharacteristic = pBatService->createCharacteristic(
        "2A19", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

    pRxCharacteristic->setCallbacks(new CharCallbacks(*this));

    bool success = pService->start();

    bool success2 = pBatService->start();

    initOTA();
    startAdvertising();
    _initialized = true;
}

void BLEController::initOTA()
{
    BLEService *otaService = pServer->createService(OTA_SERVICE_UUID);
    otaTX = otaService->createCharacteristic(
        OTA_UUID_TX, NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ);
    otaRX = otaService->createCharacteristic(
        OTA_UUID_RX, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ |
                         NIMBLE_PROPERTY::WRITE_NR);
    otaRX->setCallbacks(new OtaCallbacks(*this));
    otaTX->setCallbacks(new OtaCallbacks(*this));
    otaService->start();
    SpiffsFile::initSPIFFS();
}

void BLEController::processOTAData()
{
    if (msgota.empty())
    {
        return;
    }

    std::string data = msgota.front();
    msgota.pop();

    uint8_t msgId;
    if (data[0] == 0x01)
    {
        msgId = data[1];
        String jsonData = data.substr(1).c_str();
        handleOTA(jsonData, msgId);
    }
    else if (data[0] == 0x02)
    {
        msgId = data[1];
        std::string fileData = data.substr(1);
        SpiffsFile::ErrorCode errorCode = SpiffsFile::writeFile(fileData);
        updateFileTransferProgress(fileData.length());
        sendResponseBinaryOTA((byte)errorCode, msgId);
    }
}


void BLEController::sendResponseBinaryOTA(byte data, uint8_t msgId)
{
    // String response = String('\02') + String(char(msgId)) + String(char(data));
    String response = '\02' + String(char(data));
    sendmsgOTA(response.c_str());
}


void BLEController::sendResponseJsonOTA(String data, String msgtype, uint8_t msgId)
{
    JsonDocument Jsonresponse;
    Jsonresponse["msgtype"] = msgtype;
    Jsonresponse["response"] = data;

    String response;
    serializeJson(Jsonresponse, response);

    // String finalResponse = '\01' + byte(msgId) + response;
    String finalResponse = '\01' + response;
    sendmsgOTA(finalResponse.c_str());
}


void BLEController::sendmsgOTA(String str)
{
    str = str + '\04';
    int mtu = BLEDevice::getMTU();
    int cunkmtu = mtu - 3;

    for (int i = 0; i < str.length(); i += cunkmtu)
    {
        String chunk = str.substring(i, i + cunkmtu);
        Serial.println("--" + chunk);
        otaTX->setValue((uint8_t *)chunk.c_str(), chunk.length());
        otaTX->notify();
        delay(10);
    }
}



// void BLEController::startAdvertising() {
//     BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
//     BLEAdvertisementData adData;
//     adData.setName(this->deviceName.c_str());
//     setManufacturerData(adData);

//     adData.setCompleteServices(NimBLEUUID(heliosUUID.c_str()));
//     pAdvertising->setAdvertisementData(adData);
//     bool success = BLEDevice::startAdvertising();
//     if (success) LLOGI("BLE advertising started");
//     else LLOGI("Failed to start BLE advertising");
// }
void BLEController::startAdvertising()
{
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    BLEAdvertisementData adData;
    adData.setName(this->deviceName.c_str());
    setManufacturerData(adData);

    adData.setCompleteServices(NimBLEUUID(productUUID.c_str()));
    pAdvertising->setAdvertisementData(adData);
 \
    bool success = BLEDevice::startAdvertising();
}


void BLEController::setManufacturerData(BLEAdvertisementData &adData)
{
    std::vector<uint8_t> manData;
    manData.push_back(static_cast<uint8_t>(currentBatteryLevel));
    manData.push_back(currentSWVersion[0]);
    manData.push_back(currentSWVersion[1]);
    manData.push_back(currentSWVersion[2]);
    manData.push_back(255);
    manData.push_back(currentHWVersion[0]);
    manData.push_back(currentHWVersion[1]);
    manData.push_back(currentHWVersion[2]);

    adData.setManufacturerData(manData);
}


void BLEController::handleReceivedMessage(const std::string &message)
{

    if (TextMode)
    {
        if (message.find('\04') != std::string::npos) // run code
        {                                             // Control-D
            // executeTextBuffer();
            executeTextBufferFlag = true;
        }
        else if (message.find('\01') != std::string::npos) // abort code execution
        {
            // call callbackabort

            if (TextAbortCallback)
            {
                TextAbortCallback(message.c_str());
            }
        }
        else if (message.find('\02') != std::string::npos) // add data to lua queue
        {
            // call callbackabortTextAbortCallback

            if (TextQueueCallback)
            {
                TextQueueCallback(message.c_str());
            }
        }
        else if (message.find('\03') != std::string::npos) // to clear text buffer
        {
            TextBuffer = "";
        }
        else
        {
            TextBuffer += message;
        }
        //  receivedData.clear();
    }
    else
    {
        receivedData += message;

        if (receivedData.find('\n') != std::string::npos)
        {
            // INFO_PRINTLN("Received message: %s", receivedData.c_str());
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, receivedData);

            if (error)
            {
                // ERROR_PRINTLN("JSON parsing failed: %s", error.c_str());
            }

            if (!error)
            {

                const char *msgType = doc["msgtyp"];
                if (msgType)
                {
                    Message msg(msgType, doc);
                    auto highPriorityCallback = highPriorityCallbacks.find(msgType);
                    if (highPriorityCallback != highPriorityCallbacks.end())
                    {
                        processHighPriorityMessage(msg);
                    }
                    else
                    {
                        messageQueue.push(msg);
                    }
                }
            }

            receivedData.clear();
        }
    }
}

void BLEController::clearMessageQueue()
{
    while (!messageQueue.empty() && messageQueue.size() > 0)
    {
        if (messageQueue.front().type != nullptr)
        {
            // INFO_PRINTLN("DROPPING MESSAGE: %s", messageQueue.front().type.c_str());
            messageQueue.pop();
        }
        else
        {
            // INFO_PRINTLN("DROPPING NULL MESSAGE");
        }
    }
}

void BLEController::setIsBlockingOperationRunning(bool isRunning)
{
    this->isBlockingOperationRunning = isRunning;
}

bool BLEController::getIsBlockingOperationRunning()
{
    return this->isBlockingOperationRunning;
}

void BLEController::processHighPriorityMessage(Message &msg)
{
    auto callback = highPriorityCallbacks.find(msg.type);
    if (callback != highPriorityCallbacks.end())
    {
        callback->second(msg.doc);
    }
}

void BLEController::processJsonMessages()
{

    while (messageQueue.empty() == false && this->isBlockingOperationRunning == false)
    {
        // INFO_PRINTLN("BLEController::processMessages()");
        Message &msg = messageQueue.front();
        auto callback = messageCallbacks.find(msg.type);
        if (callback != messageCallbacks.end())
        {
            this->setIsBlockingOperationRunning(true);
            // //INFO_PRINTLN("BLEController::processMessages() callback: %s", msg.type.c_str());
            // create copy of message
            // JsonDocument doc = msg.doc;

            callback->second(msg.doc);
            // //INFO_PRINTLN("BLEController::processMessages() callback done: %s", msg.type.c_str());
            if (this->isBlockingOperationRunning == true && messageQueue.empty() == false)
                messageQueue.pop(); // pop processed message

            this->setIsBlockingOperationRunning(false);
        }
        else if (messageQueue.empty() == false) // pop unwanted messages
        {
            messageQueue.pop();
        }

        // INFO_PRINTLN("BLEController::processMessages() done");
    }
}

void BLEController::processTextMessages()
{

    if (TextMode && executeTextBufferFlag)
    {
        executeTextBufferFlag = false;

        executeTextBuffer();
    }
}

void BLEController::sendMessage(const JsonDocument &message)
{
    String jsonString;
    serializeJson(message, jsonString);
    jsonString += '\n'; // Add newline as message delimiter

    int sendMtu = mtu - 3; // 3 bytes overhead
    // INFO_PRINTLN("Sending BLE message: %s", jsonString.c_str());
    for (size_t i = 0; i < jsonString.length(); i += sendMtu)
    {
        String chunk = jsonString.substring(i, std::min(i + sendMtu, jsonString.length()));
        pTxCharacteristic->setValue((uint8_t *)chunk.c_str(), chunk.length());
        pTxCharacteristic->notify();
        // delay(10); // Give the client time to process
        delayMicroseconds(100); // Give the client time to process
    }
}

void BLEController::sendMessage(String jsonString)
{

    jsonString += '\n'; // Add newline as message delimiter

    int sendMtu = mtu - 3; // 3 bytes overhead
    // INFO_PRINTLN("Sending BLE message: %s", jsonString.c_str());
    for (size_t i = 0; i < jsonString.length(); i += sendMtu)
    {
        String chunk = jsonString.substring(i, std::min(i + sendMtu, jsonString.length()));
        pTxCharacteristic->setValue((uint8_t *)chunk.c_str(), chunk.length());
        pTxCharacteristic->notify();
        //    delay(1); // Give the client time to process
        delayMicroseconds(100); // Give the client time to process
    }
}

void BLEController::registerMessageCallback(const String &msgType, std::function<void(JsonDocument &)> callback)
{
    messageCallbacks[msgType] = callback;
    // INFO_PRINTLN("Registered message callback: %s", msgType.c_str());
}

void BLEController::registerHighPriorityCallback(const String &msgType, std::function<void(JsonDocument &)> callback)
{

    highPriorityCallbacks[msgType] = callback;
    // INFO_PRINTLN("Registered high priority message callback: %s", msgType.c_str());
}

void BLEController::updateBatteryAndVersion(int batteryLevel, const uint8_t *version, size_t SWversionLength, uint8_t *HWVersion, size_t HWVersionLength)
{
    currentBatteryLevel = batteryLevel;
    if (SWversionLength > sizeof(currentSWVersion))
    {
        SWversionLength = sizeof(currentSWVersion);
    }
    memcpy(currentSWVersion, version, SWversionLength);

    if (HWVersionLength > sizeof(currentHWVersion))
    {
        HWVersionLength = sizeof(currentHWVersion);
    }
    memcpy(currentHWVersion, HWVersion, HWVersionLength);

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    BLEAdvertisementData adData;
    adData.setName(this->deviceName.c_str());
    setManufacturerData(adData);
    adData.setCompleteServices(NimBLEUUID(productUUID.c_str()));
    pAdvertising->setAdvertisementData(adData);

    if (pServer->getAdvertising()->isAdvertising())
    {
        pServer->getAdvertising()->stop();
        pServer->getAdvertising()->start();
    }

    pBatCharacteristic->setValue((uint8_t *)&batteryLevel, 1);
    pBatCharacteristic->notify();
    // INFO_PRINTLN("Updated battery level: %d", batteryLevel);
    // INFO_PRINTLN("Updated version: %d.%d.%d", currentSWVersion[0], currentSWVersion[1], currentSWVersion[2]);
    // INFO_PRINTLN("Updated HW version: %d.%d.%d", currentHWVersion[0], currentHWVersion[1], currentHWVersion[2]);
}

void BLEController::updateBattery(int batteryLevel)
{
    currentBatteryLevel = batteryLevel;
    pBatCharacteristic->setValue((uint8_t *)&batteryLevel, 1);
    pBatCharacteristic->notify();
}

BLEController::FileTransferStatus BLEController::getFileTransferStatus()
{
    return fileStatus;
}

void BLEController::updateFileTransferProgress(size_t bytesReceived)
{
    fileStatus.bytesTransferred += bytesReceived;
    fileStatus.progressPercentage = (fileStatus.bytesTransferred * 100) / fileStatus.totalSize;
    // You might want to send a progress update to the client here
}

void BLEController::resetFileTransferStatus()
{
    fileStatus = {};
}
void BLEController::handleOTA(String data, uint8_t msgId)
{
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, data);
    String response;

    if (error)
    {
        Serial.println("Failed to parse JSON");
        sendResponseJsonOTA("JSON_ERROR", "jsonerror", msgId);
        return;
    }

    const char *msgtype = doc["msgtype"];
    if (strcmp(msgtype, "fileopen") == 0)
    {
        const char *filename = doc["filename"];
        size_t size = doc["filesize"];
        fileStatus.currentFileName = doc["filename"].as<String>();
        fileStatus.fileIndex = fileStatus.fileIndex;
        fileStatus.totalSize = doc["filesize"].as<size_t>();
        SpiffsFile::ErrorCode errorCode = SpiffsFile::createFile(filename, size);
        response = SpiffsFile::errorCodeToString(errorCode);
        sendResponseJsonOTA(response, msgtype, msgId);
    }
    else if (strcmp(msgtype, "fileclose") == 0)
    {
        SpiffsFile::ErrorCode errorCode = SpiffsFile::closeFile();
        resetFileTransferStatus();
        fileStatus.fileIndex++;
        response = SpiffsFile::errorCodeToString(errorCode);
        sendResponseJsonOTA(response, msgtype, msgId);
    }
    else if (strcmp(msgtype, "filedelete") == 0)
    {
        const char *filename = doc["filename"];
        SpiffsFile::ErrorCode errorCode = SpiffsFile::deleteFile(filename);
        response = SpiffsFile::errorCodeToString(errorCode);
        sendResponseJsonOTA(response, msgtype, msgId);
    }
    else if (strcmp(msgtype, "filerename") == 0)
    {
        const char *oldFilename = doc["oldfilename"];
        const char *newFilename = doc["newfilename"];
        SpiffsFile::ErrorCode errorCode = SpiffsFile::renameFile(oldFilename, newFilename);
        response = SpiffsFile::errorCodeToString(errorCode);
        sendResponseJsonOTA(response, msgtype, msgId);
    }
    else if (strcmp(msgtype, "filelist") == 0)
    {
        Serial.println("filelist");
        JsonDocument responseDoc; // Increase the size as needed
        JsonArray fileListArray = responseDoc.createNestedArray("fileList");
        SpiffsFile::ErrorCode errorCode = SpiffsFile::listFiles(fileListArray);

        if (errorCode == SpiffsFile::FILE_OK)
        {
            responseDoc["msgtype"] = "filelist";
            responseDoc["response"] = "OK";

            String response;
            serializeJson(responseDoc, response);
            sendResponseJsonOTA(response, msgtype, msgId);
        }
        else
        {
            String errorStr = SpiffsFile::errorCodeToString(errorCode);
            sendResponseJsonOTA(errorStr, msgtype, msgId);
        }
    }

    else if (strcmp(msgtype, "formatspiffs") == 0)
    {
        SpiffsFile::ErrorCode errorCode = SpiffsFile::formatSPIFFS();
        response = SpiffsFile::errorCodeToString(errorCode);
        sendResponseJsonOTA(response, msgtype, msgId);
    }
    else if (strcmp(msgtype, "initspiffs") == 0)
    {
        SpiffsFile::ErrorCode errorCode = SpiffsFile::initSPIFFS();
        response = SpiffsFile::errorCodeToString(errorCode);
        sendResponseJsonOTA(response, msgtype, msgId);
    }
    else if (strcmp(msgtype, "otaupdate") == 0)
    {
        const char *filename = doc["filename"];
        bool success = updateFromFS(SPIFFS, filename);
        if (success)
        {
            sendResponseJsonOTA("OTA update successful", msgtype, msgId);
            delay(1000);
            ESP.restart();
        }
        else
        {
            sendResponseJsonOTA("OTA update failed", msgtype, msgId);
        }
    }
    else if (strcmp(msgtype, "spiffssize") == 0)
    {
        size_t totalBytes = SPIFFS.totalBytes();
        size_t usedBytes = SPIFFS.usedBytes();
        size_t freeBytes = totalBytes - usedBytes;
        JsonDocument responseDoc;
        responseDoc["totalBytes"] = totalBytes;
        responseDoc["usedBytes"] = usedBytes;
        responseDoc["freeBytes"] = freeBytes;
        serializeJson(responseDoc, response);
        sendResponseJsonOTA(response, msgtype, msgId);
    }
}

void BLEController::setOtaCallbacks(std::function<void()> startCallback,
                                    std::function<void(int)> progressCallback,
                                    std::function<void()> successCallback,
                                    std::function<void(const char *)> errorCallback)
{
    otaStartCallback = startCallback;
    otaProgressCallback = progressCallback;
    otaSuccessCallback = successCallback;
    otaErrorCallback = errorCallback;
}

void BLEController::setServiceUUID(const char *uuid)
{
    serviceUUID = uuid;
}

void BLEController::setCharacteristicUUIDs(const char *txUuid, const char *rxUuid)
{
    txUUID = txUuid;
    rxUUID = rxUuid;
}

void BLEController::setOnConnectCallback(std::function<void()> callback)
{
    onConnectCallback = callback;
}

void BLEController::setOnDisconnectCallback(std::function<void()> callback)
{
    onDisconnectCallback = callback;
    startAdvertising();
    // bleController.switchToJsonMode();
}

void BLEController::stop()
{

    // INFO_PRINTLN("BLEController::stop()");
    if (!_initialized)
        return;

    // Stop advertising if it's currently running
    if (BLEDevice::getAdvertising()->isAdvertising())
    {
        BLEDevice::getAdvertising()->stop();
    }

    // Disconnect any connected devices
    if (pServer != nullptr)
    {
        // for (auto client : pServer->getPeerDevices(true))
        // {
        //     pServer->disconnect(client);
        // }
    }

    // Stop all BLE services
    if (pServer != nullptr)
    {
        // for (auto service : pServer->getServices())
        // {
        //     service->stop();
        // }
    }

    // Clear all characteristics
    // if (pTxCharacteristic != nullptr)
    // {
    //     pTxCharacteristic->removeDescriptor(nullptr);
    //     pTxCharacteristic = nullptr;
    // }
    // if (pRxCharacteristic != nullptr)
    // {
    //     pRxCharacteristic->removeDescriptor(nullptr);
    //     pRxCharacteristic = nullptr;
    // }
    // if (pBatCharacteristic != nullptr)
    // {
    //     pBatCharacteristic->removeDescriptor(nullptr);
    //     pBatCharacteristic = nullptr;
    // }
    // if (otaTX != nullptr)
    // {
    //     otaTX->removeDescriptor(nullptr);
    //     otaTX = nullptr;
    // }
    // if (otaRX != nullptr)
    // {
    //     otaRX->removeDescriptor(nullptr);
    //     otaRX = nullptr;
    // }

    // Clear all callbacks
    messageCallbacks.clear();
    highPriorityCallbacks.clear();
    onConnectCallback = nullptr;
    onDisconnectCallback = nullptr;
    otaStartCallback = nullptr;
    otaProgressCallback = nullptr;
    otaSuccessCallback = nullptr;
    otaErrorCallback = nullptr;

    // Clear message queue
    clearMessageQueue();

    // Reset all state variables
    deviceConnected = false;
    isBlockingOperationRunning = false;
    currentBatteryLevel = 0;
    memset(currentSWVersion, 0, sizeof(currentSWVersion));

    // Delete the BLE server
    if (pServer != nullptr)
    {
        // BLEDevice::deleteServer();
        pServer = nullptr;
    }

    // Finally, deinitialize the BLE device
    BLEDevice::deinit(true);

    // INFO_PRINTLN("BLEController::stop() complete");
}

void BLEController::switchToTextMode()
{
    TextMode = true;
    executeTextBufferFlag = false;
    TextBuffer.clear();
    sendMessage("Entered Text mode. Use Control-D to execute.");

    // Register BLE-specific Text functions
}

void BLEController::switchToJsonMode()
{
    TextMode = false;
    TextBuffer.clear();
    JsonDocument doc;
    doc["msgtyp"] = "jsonmode";
    sendMessage(doc);
}

void BLEController::executeTextBuffer()
{
    textBufferIsExecuting = true;
    if (TextMessageCallback != nullptr)
    {
        TextMessageCallback(TextBuffer.c_str());
    }

    textBufferIsExecuting = false;
    executeTextBufferFlag = false;

    TextBuffer.clear();
}

void BLEController::sendTextOutput(const std::string &_output)
{
    String output = _output.c_str();
    output = output + '\n';
    int sendMtu = mtu - 3; // 3 bytes overhead
    // INFO_PRINTLN("Sending BLE message: %s", output.c_str());
    for (size_t i = 0; i < output.length(); i += sendMtu)
    {
        String chunk = output.substring(i, std::min(i + sendMtu, output.length()));
        pTxCharacteristic->setValue((uint8_t *)chunk.c_str(), chunk.length());
        pTxCharacteristic->notify();
        //    delay(1); // Give the client time to process
        delayMicroseconds(100); // Give the client time to process
    }
}

bool BLEController::transferFileFromSPIFFS(const char *filename, uint16_t mtu)
{
    if (!SPIFFS.begin(true))
    {
        return false;
    }

    File file = SPIFFS.open(filename, "r");
    if (!file)
    {
        SPIFFS.end();
        return false;
    }

    uint8_t buffer[512];      // Or whatever maximum size you want to support
    size_t sendMtu = mtu - 3; // 3 bytes overhead

    while (file.available())
    {
        size_t bytesRead = file.read(buffer, sendMtu);
        if (bytesRead == 0)
        {
            break;
        }

        pTxCharacteristic->setValue(buffer, bytesRead);
        pTxCharacteristic->notify();
        delay(20);
    }

    file.close();
    SPIFFS.end();
    return true;
}
void initializeBLEController(String deviceName) {
    bleController.begin(deviceName);
    bleController.setOnConnectCallback(onBLEConnect);
    bleController.setOnDisconnectCallback(onBLEDisconnect);
    bleController.setOtaCallbacks(onOtaStart, onOtaProgress, onOtaSuccess, onOtaError);
    bleController.setTextMessageCallback(lua_loop);
    bleController.setTextAbortCallback(luaClose);
    // bleController.setTextQueueCallback(addStringToQueue);
    // Register BLE message handlers
    initializeBLEHandlers();
    // initQueue();

    INFO_PRINTLN("BLE Controller initialized");
    
    LLOGI("BLE Controller initialized end");
}
    


//  #include <NimBLEDevice.h>
//  #include <Arduino.h>
// extern "C"
// {
// #define LUAT_LOG_TAG "BLE_CTRL"
// #include "luat_log.h"
// }

//  void setManufacturerData(BLEAdvertisementData &adData);

//  static NimBLEServer* pServer;
//  BLECharacteristic *pRxCharacteristic;
//  BLECharacteristic *pTxCharacteristic;
//  /**  None of these are required as they will be handled by the library with defaults. **
//   **                       Remove as you see fit for your needs                        */
//  class ServerCallbacks : public NimBLEServerCallbacks {
//      void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
//          LLOGI("Client address: %s\n", connInfo.getAddress().toString().c_str());
 
//          /**
//           *  We can use the connection handle here to ask for different connection parameters.
//           *  Args: connection handle, min connection interval, max connection interval
//           *  latency, supervision timeout.
//           *  Units; Min/Max Intervals: 1.25 millisecond increments.
//           *  Latency: number of intervals allowed to skip.
//           *  Timeout: 10 millisecond increments.
//           */
//          pServer->updateConnParams(connInfo.getConnHandle(), 24, 48, 0, 180);
//      }
 
//      void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
//         LLOGI("Client disconnected - start advertising\n");
//          NimBLEDevice::startAdvertising();
//      }
 
//      void onMTUChange(uint16_t MTU, NimBLEConnInfo& connInfo) override {
//         LLOGI("MTU updated: %u for connection ID: %u\n", MTU, connInfo.getConnHandle());
//      }
 
//      /********************* Security handled here *********************/
//      uint32_t onPassKeyDisplay() override {
//         LLOGI("Server Passkey Display\n");
//          /**
//           * This should return a random 6 digit number for security
//           *  or make your own static passkey as done here.
//           */
//          return 123456;
//      }
 
//      void onConfirmPassKey(NimBLEConnInfo& connInfo, uint32_t pass_key) override {
//         LLOGI("The passkey YES/NO number: %" PRIu32 "\n", pass_key);
//          /** Inject false if passkeys don't match. */
//          NimBLEDevice::injectConfirmPasskey(connInfo, true);
//      }
 
//      void onAuthenticationComplete(NimBLEConnInfo& connInfo) override {
//          /** Check that encryption was successful, if not we disconnect the client */
//          if (!connInfo.isEncrypted()) {
//              NimBLEDevice::getServer()->disconnect(connInfo.getConnHandle());
//              LLOGI("Encrypt connection failed - disconnecting client\n");
//              return;
//          }
 
//          LLOGI("Secured connection to: %s\n", connInfo.getAddress().toString().c_str());
//      }
//  } serverCallbacks;
 
//  /** Handler class for characteristic actions */
//  class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
//      void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
//         LLOGI("%s : onRead(), value: %s\n",
//                 pCharacteristic->getUUID().toString().c_str(),
//                 pCharacteristic->getValue().c_str());
//      }
 
//      void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
//         LLOGI("%s : onWrite(), value: %s\n",
//                 pCharacteristic->getUUID().toString().c_str(),
//                 pCharacteristic->getValue().c_str());
//      }
 
//      /**
//       *  The value returned in code is the NimBLE host return code.
//       */
//      void onStatus(NimBLECharacteristic* pCharacteristic, int code) override {
//         LLOGI("Notification/Indication return code: %d, %s\n", code, NimBLEUtils::returnCodeToString(code));
//      }
 
//      /** Peer subscribed to notifications/indications */
//      void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) override {
//          std::string str  = "Client ID: ";
//          str             += connInfo.getConnHandle();
//          str             += " Address: ";
//          str             += connInfo.getAddress().toString();
//          if (subValue == 0) {
//              str += " Unsubscribed to ";
//          } else if (subValue == 1) {
//              str += " Subscribed to notifications for ";
//          } else if (subValue == 2) {
//              str += " Subscribed to indications for ";
//          } else if (subValue == 3) {
//              str += " Subscribed to notifications and indications for ";
//          }
//          str += std::string(pCharacteristic->getUUID());
 
//          LLOGI("%s\n", str.c_str());
//      }
//  } chrCallbacks;
 
//  /** Handler class for descriptor actions */
//  class DescriptorCallbacks : public NimBLEDescriptorCallbacks {
//      void onWrite(NimBLEDescriptor* pDescriptor, NimBLEConnInfo& connInfo) override {
//          std::string dscVal = pDescriptor->getValue();
//          LLOGI("Descriptor written value: %s\n", dscVal.c_str());
//      }
 
//      void onRead(NimBLEDescriptor* pDescriptor, NimBLEConnInfo& connInfo) override {
//         LLOGI("%s Descriptor read\n", pDescriptor->getUUID().toString().c_str());
//      }
//  } dscCallbacks;
 
// void initializeBLEController() {
//     NimBLEDevice::init("NimBLE");
 
//     BLEDevice::setPower(7);
//     BLEDevice::setMTU(512);
//     pServer = NimBLEDevice::createServer();
//     pServer->setCallbacks(&serverCallbacks);

//     NimBLEService*        pService = pServer->createService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
//     pTxCharacteristic = pService->createCharacteristic("6E400003-B5A3-F393-E0A9-E50E24DCCA9E", NIMBLE_PROPERTY::NOTIFY);
//    if (!pTxCharacteristic) {
//        printf("Failed to create TX characteristic");
//        return;
//    }
//    pRxCharacteristic = pService->createCharacteristic("6E400002-B5A3-F393-E0A9-E50E24DCCA9E", NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ);
//    if (!pRxCharacteristic) {
//        printf("Failed to create RX characteristic");
//        return;
//    }
//      pRxCharacteristic->setCallbacks(&chrCallbacks);

//     pService->start();

//     NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
//     BLEAdvertisementData adData;
//    adData.setName("Helios");
//    adData.setCompleteServices(NimBLEUUID("AE05"));
//    pAdvertising->setAdvertisementData(adData);
//    bool success = BLEDevice::startAdvertising();
//    LLOGI("Advertising Started\n");


// }
    