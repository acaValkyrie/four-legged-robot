#include <Arduino.h>
#include <Servo.h>
#include <BLEDevice.h>

#define Joycon_address "98:B6:E9:89:8A:34"
// 検索するBLEデバイス。serviceUUIDを調べる場合には空にする(例はHuman Interface Device"00001812-0000-1000-8000-00805f9b34fb")
static BLEUUID serviceUUID("");
static BLEAdvertisedDevice* myDevice;
// 接続してCharacteristic一覧を取得
bool connectToServer() {
  USBSerial.print("接続先 : ");
  USBSerial.println(myDevice->getAddress().toString().c_str());
  BLEClient*  pClient  = BLEDevice::createClient();
  pClient->connect(myDevice);
  // サービス取得
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    pClient->disconnect();
    return false;
  }
  // Characteristic一覧
  USBSerial.println("characteristic一覧");
  std::map<std::string, BLERemoteCharacteristic*>* mapCharacteristics = pRemoteService->getCharacteristics();
  for (std::map<std::string, BLERemoteCharacteristic*>::iterator i = mapCharacteristics->begin(); i != mapCharacteristics->end(); ++i) {
    USBSerial.print(" - characteristic UUID : ");
    USBSerial.print(i->first.c_str());
    USBSerial.print(" Broadcast:");
    USBSerial.print(i->second->canBroadcast()?'O':'X');
    USBSerial.print(" Read:");
    USBSerial.print(i->second->canRead()?'O':'X');
    USBSerial.print(" WriteNoResponse:");
    USBSerial.print(i->second->canWriteNoResponse()?'O':'X');
    USBSerial.print(" Write:");
    USBSerial.print(i->second->canWrite()?'O':'X');
    USBSerial.print(" Notify:");
    USBSerial.print(i->second->canNotify()?'O':'X');
    USBSerial.print(" Indicate:");
    USBSerial.print(i->second->canIndicate()?'O':'X');
    USBSerial.println();
  }
  // stop
  USBSerial.println("プログラム停止!");
  while (1) delay(1000);
}
// 検索したデバイスを受信するコールバック関数
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      USBSerial.print("BLE デバイス発見 : ");
      USBSerial.println(advertisedDevice.toString().c_str());
      if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
        // 指定デバイスだったら接続する
        BLEDevice::getScan()->stop();
        myDevice = new BLEAdvertisedDevice(advertisedDevice);
      }
    }
};

const int servoNum = 4;
const int servoPins[servoNum] = {5, 6, 7, 1};
Servo servos[servoNum];

void setup() {
    // Serial.begin(115200);
    USBSerial.begin(9600);

    delay(1000);

    BLEDevice::init("");
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(5, false);

    // for(int i = 0; i < servoNum; i++){
    //     pinMode(servoPins[i], OUTPUT);
    //     servos[i].attach(servoPins[i], 853, 2090);
    //     servos[i].write(servoPins[i], 90);
    // }
    // delay(500);
    // Serial.print("bluetooth_connected\n");
}

void f(int index, int degree){
    servos[index].write(servoPins[index], degree);
    delay(100);
}

void loop(){
    if (myDevice != NULL) {
        connectToServer();
    }
    delay(1000);
    // int delta = 60;
    // for(int i = 0; i < servoNum; i++) f(i, 90-delta);
    // for(int i = 0; i < servoNum; i++) f(i, 90);
    // for(int i = 0; i < servoNum; i++) f(i, 90+delta);
    // for(int i = 0; i < servoNum; i++) f(i, 90);
}

