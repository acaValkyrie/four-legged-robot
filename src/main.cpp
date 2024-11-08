#include <Arduino.h>
#include <Servo.h>

const int servoNum = 4;
const int servoPins[servoNum] = {5, 6, 7, 1};
Servo servos[servoNum];

void setup() {
    Serial.begin(115200);
    for(int i = 0; i < servoNum; i++){
        pinMode(servoPins[i], OUTPUT);
        servos[i].attach(servoPins[i], 853, 2090);
        servos[i].write(servoPins[i], 90);
    }
    delay(500);
    Serial.print("bluetooth_connected\n");
}

void f(int index, int degree){
    servos[index].write(servoPins[index], degree);
    delay(100);
}

void loop(){
    int delta = 60;
    for(int i = 0; i < servoNum; i++) f(i, 90-delta);
    for(int i = 0; i < servoNum; i++) f(i, 90);
    for(int i = 0; i < servoNum; i++) f(i, 90+delta);
    for(int i = 0; i < servoNum; i++) f(i, 90);
}