#include <Arduino.h>
#include <myBLE.hpp>
#include <myServo.hpp>

void BLETask(void *arg);
void servoTask(void *arg);

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  pinMode(BLEConnectIndicatorLEDPin, OUTPUT);
  setupServo();
  
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);

  xTaskCreatePinnedToCore(BLETask,   "BLETask",   4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(servoTask, "servoTask", 4096, NULL, 2, NULL, 1);
}  // End of setup.
 
void loop() {delay(1000);}

void BLETask(void *arg){
  while(1){
    // If the flag "doConnect" is true then we have scanned for and found the desired
    // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
    // connected we set the connected flag to be true.
    if (doConnect == true) {
      if (connectToServer()) {
        Serial.println("We are now connected to the BLE Server.");
      } else {
        Serial.println("We have failed to connect to the server; there is nothing more we will do.");
      }
      doConnect = false;
    }
    
    // If we are connected to a peer BLE Server, update the characteristic each time we are reached
    // with the current time since boot.
    if (connected) {

      // Set the characteristic's value to be the array of bytes that is actually a string.
      

    } else if (doScan) {
      BLEDevice::getScan()->start(0);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
    }

    delay(1000);  // Delay a second between loops.
  }
}

int angle = 0;
int angleStep = 10;
void servoTask(void *arg){
  while(1){
    if(stick_y == 1){
      angle += angleStep;
      if(angle > 90) angle = 90;
    }else if(stick_y == -1){
      angle -= angleStep;
      if(angle < -90) angle = -90;
    }
    setServoAngle(FRONT_RIGHT, angle);
    delay(100);
  }
}