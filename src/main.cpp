#include <Arduino.h>
#include <ESP32Servo.h>

const int servoPin = 26;
Servo servo;

void setup() {
    Serial.begin(115200);
    pinMode(servoPin, OUTPUT);
    servo.setPeriodHertz(50);
    servo.attach(servoPin, 853, 2090);
    servo.write(0);
}

void f(int degree){
    Serial.print("Servo is at ");
    Serial.print(degree);
    Serial.println(" degrees");
    servo.write(degree);
    delay(1000);
}

void loop(){
    int delta = 60;
    f(0);
    f(90);
    f(180);
    f(90);
}