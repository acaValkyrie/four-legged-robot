#pragma once
/**
 * A BLE client example that is rich in capabilities.
 * There is a lot new capabilities implemented.
 * author unknown
 * updated by chegewara
 */

#include <Arduino.h>
#include "BLEDevice.h"
//#include "BLEScan.h"

// The remote service we wish to connect to.
static BLEUUID HIDserviceUUID((uint16_t)0x1812);
// The characteristic of the remote service we are interested in.
static BLEUUID ReportCharUUID("00002A4D-0000-1000-8000-00805F9B34FB");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic *pRemoteCharacteristic;
static BLEAdvertisedDevice *myDevice;
const int BLEConnectIndicatorLEDPin = 4;
#define TARGET_DEVICE_NAME "VR-PARK"
static int stick_x = 0;
static int stick_y = 0;

static void notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify) {
  const int joy_x_index = 1;
  const int joy_y_index = 2;
  int x = pData[joy_x_index];
  int y = pData[joy_y_index];
  const int X_RIGHT = 15;  // 0000 1111
  const int X_LEFT  = 239; // 1110 1111
  const int Y_UP    = 239; // 1110 1111
  const int Y_DOWN  = 15;  // 0000 1111
  
  if(x == X_RIGHT) stick_x = 1;
  else if(x == X_LEFT) stick_x = -1;
  else stick_x = 0;

  if(y == Y_UP) stick_y = 1;
  else if(y == Y_DOWN) stick_y = -1;
  else stick_y = 0;
  Serial.printf("x: %d, y: %d\n", stick_x, stick_y);
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient *pclient) {  
    digitalWrite(BLEConnectIndicatorLEDPin, HIGH);
  }

  void onDisconnect(BLEClient *pclient) {
    connected = false;
    Serial.println("onDisconnect");
    digitalWrite(BLEConnectIndicatorLEDPin, LOW);
  }
};

typedef void (*NotifyCallback)(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify);
bool setupCharacteristics(BLERemoteService *pRemoteService, BLEClient *pClient, NotifyCallback pNotifyCallback) {
  // Obtain a reference to the characteristic in the service of the remote BLE server.
  std::map<std::uint16_t, BLERemoteCharacteristic*> *pmapbh;
  pmapbh = pRemoteService->getCharacteristicsByHandle();
  for (std::map<std::uint16_t, BLERemoteCharacteristic*>::iterator itrbh = pmapbh->begin(); itrbh != pmapbh->end(); itrbh++){
    BLEUUID uuid = itrbh->second->getUUID();
    Serial.print("Characteristic UUID: ");
    Serial.println(uuid.toString().c_str());
    
    // this uuid is report characteristic
    if(!uuid.equals(ReportCharUUID)) continue;
    
    // this characteristic can notify
    if(!itrbh->second->canNotify()) continue;
    itrbh->second->registerForNotify(pNotifyCallback);
  }
  return true;
}
 
bool connectToServer() {
  Serial.print("Forming a connection to ");
  Serial.println(myDevice->getAddress().toString().c_str());

  BLEClient *pClient = BLEDevice::createClient();
  Serial.println(" - Created client");

  pClient->setClientCallbacks(new MyClientCallback());

  // Connect to the remove BLE Server.
  pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
  Serial.println(" - Connected to server");
  pClient->setMTU(517);  //set client to request maximum MTU from server (default is 23 otherwise)

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService *pRemoteService = pClient->getService(HIDserviceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find HID service UUID: ");
    Serial.println(HIDserviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found HID service");
  setupCharacteristics(pRemoteService, pClient, notifyCallback);

  connected = true;
  return true;
}

/**
* Scan for BLE servers and find the first one that advertises the service we are looking for.
*/

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  /**
    * Called for each advertising BLE server.
    */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if ( !advertisedDevice.haveServiceUUID() ) return;
    if ( !advertisedDevice.isAdvertisingService(HIDserviceUUID) ) return;
    if ( advertisedDevice.getName()!=TARGET_DEVICE_NAME) return;
    
    BLEDevice::getScan()->stop();
    myDevice = new BLEAdvertisedDevice(advertisedDevice);
    doConnect = true;
    doScan = true;
  }  // onResult
};  // MyAdvertisedDeviceCallbacks