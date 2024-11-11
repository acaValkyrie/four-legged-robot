// Found a new device asynchronously: Name: Joy-Con (R), Address: 98:b6:e9:89:8a:34, cod: 0x002508, rssi: -72
// Found a new device asynchronously: Name: Lic Pro Controller, Address: 30:31:7d:80:19:52, cod: 0x002508, rssi: -56
#include <Arduino.h>
#include <BluetoothSerial.h>
BluetoothSerial SerialBT;
// uint8_t address[6] = {0x98, 0xb6, 0xe9, 0x89, 0x8a, 0x34};
uint8_t address[6] = {0x30, 0x31, 0x7d, 0x80, 0x19, 0x52};
const char* pin = "0000";

void setup(){
  Serial.begin(115200);
  
  if(!SerialBT.begin("ESP32", true)){
    Serial.println("An error occurred initializing Bluetooth");
    return;
  }
  SerialBT.setPin(pin);
  delay(1000);
  if(SerialBT.connect(address)){
    Serial.println("Connected to Joy-Con (R)");
  } else {
    Serial.println("Couldn't connect to Joy-Con (R)");
  }
}

void loop(){
  if(SerialBT.available()){
    Serial.write(SerialBT.read());
  }
  if(Serial.available()){
    SerialBT.write(Serial.read());
  }
}