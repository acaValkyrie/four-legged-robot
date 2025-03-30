#pragma once

#include <Arduino.h>
#include <myBLE.hpp>
#include <ESP32Servo.h>

enum ServoType {
  FRONT_LEFT,
  FRONT_RIGHT,
  BACK_LEFT,
  BACK_RIGHT,
  WAIST
};

#define SERVO_NUM 5
#define M1 5
#define M2 1
#define M3 6
#define M4 3
#define M5 10
const int servoPins[SERVO_NUM] = {M1, M2, M4, M5, M3};
Servo servos[SERVO_NUM];

void setupServo(){
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    ESP32PWM::allocateTimer(4);
    const int minUs = 853;
    const int maxUs = 1997;
    for(int i = 0; i < SERVO_NUM; i++){
        int ret = servos[i].attach(servoPins[i], minUs, maxUs);
        Serial.print("Servo attached to pin ");
        Serial.println(servoPins[i]);
        Serial.print("Channel: ");
        Serial.println(ret);

    }
}

void setServoAngle(ServoType type, int angle){
    angle = angle + 90;
    if (type == FRONT_LEFT || type == BACK_LEFT){
        angle = 180 - angle;
    }
    Serial.print("Servo ");
    Serial.print(type);
    Serial.print(" angle: ");
    Serial.println(angle);
    servos[type].write(angle);
}

void setAllServoAngle(int angle){
    for(int i = 0; i < SERVO_NUM; i++){
        setServoAngle((ServoType)i, angle);
    }
}

void setServoFree(ServoType type){
    servos[type].detach();
}

void setAllServoFree(){
    for(int i = 0; i < SERVO_NUM; i++){
        servos[i].detach();
    }
}

