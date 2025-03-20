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
    } else if (doScan) {
      BLEDevice::getScan()->start(0);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
    }

    delay(100);  // Delay a second between loops.
  }
}

void servoTask(void *arg){
  enum state{
    FORWARD,
    FORWARD_RIGHT,
    RIGHT,
    BACKWARD_RIGHT,
    BACKWARD,
    BACKWARD_LEFT,
    LEFT,
    FORWARD_LEFT,
    STOP
  };
  state currentState = STOP;

  int theta = 0;
  const int thetaStep = 10;
  while(1){
    if (stick_y == 1){
      theta+=thetaStep;
    }else if(stick_y == -1){
      theta-=thetaStep;
    }
    int arm_amp = 60;
    for(ServoType type = FRONT_LEFT; type <= BACK_RIGHT; type = (ServoType)(type+1)){
      int phi = 0;
      if (type == FRONT_LEFT || type == BACK_RIGHT){
        phi = 180;
      }
      int angle = arm_amp*sin(theta*PI/180 + phi*PI/180);
      setServoAngle(type, angle);
    }
    int waist_amp = 30;
    int angle = waist_amp*sin(theta*PI/180 + 180*PI/180);
    setServoAngle(WAIST, angle);
    delay(100);
  }
}