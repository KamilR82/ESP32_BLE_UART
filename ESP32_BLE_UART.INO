// ESP32 Bluetooth low energy (BLE) to UART Bridge Simple Example

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>  // Notifications & Indications // https://github.com/espressif/arduino-esp32/blob/master/libraries/BLE/src/BLE2902.h

BLECharacteristic *pTxCharacteristic;
BLECharacteristic *pRxCharacteristic;

//Nordic nRF:
const char *BLE_UART_UUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";//Bluetooth LE GATT UUIDs for the Nordic UART profile
const char *BLE_RX_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";//Nordic UART Service write
const char *BLE_TX_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e";//Nordic UART Service notify

bool BLEclientConnected = false;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    BLEclientConnected = true;
    Serial.println("BLE client has connected.");
  };

  void onDisconnect(BLEServer *pServer) {
    BLEclientConnected = false;
    Serial.println("BLE client has disconnected.");
    Serial.print("BLE advertising restart");
    pServer->startAdvertising();                               // restart advertising
    Serial.println("ed - Waiting on a client connection...");  // restart+ed
  }
};

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    auto recieved = pCharacteristic->getValue();
    if (recieved.length() > 0) {
      Serial.print("Received: ");
      Serial.println(recieved);
    }
  }
};

void BLEsend(const char *str) {
  if (BLEclientConnected) {
    pTxCharacteristic->setValue(str);
    pTxCharacteristic->notify(true);
    delay(10);
  }
}

void BLEbegin() {
  // Create the BLE Device
  BLEDevice::init("BLE UART");

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(BLEUUID(BLE_UART_UUID));

  // Create a BLE Characteristic
  pRxCharacteristic = pService->createCharacteristic(BLEUUID(BLE_RX_UUID), BLECharacteristic::PROPERTY_WRITE);
  pTxCharacteristic = pService->createCharacteristic(BLEUUID(BLE_TX_UUID), BLECharacteristic::PROPERTY_NOTIFY);  // | BLECharacteristic::PROPERTY_READ

  // Create a BLE Descriptor 0x2902: Client Characteristic Configuration Descriptor (CCCD)
  pTxCharacteristic->addDescriptor(new BLE2902());
  pRxCharacteristic->addDescriptor(new BLE2902());

  pTxCharacteristic->setReadProperty(true);
  pRxCharacteristic->setWriteProperty(true);
  pRxCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();  // Start the service

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(BLEUUID(BLE_UART_UUID));
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x12);  // help with iPhone connections issue (use 0x06 or 0x12, default is 0x20)
  pAdvertising->start();
  Serial.println("BLE advertising started - Waiting on a client connection...");
}

void setup() {
  Serial.begin(115200);
  BLEbegin();
}

void loop() {
  BLEsend("Hello World\n");
  delay(2000);
}
