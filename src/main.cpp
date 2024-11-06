#include <Arduino.h>
#include <FastLED.h>
#include <Servo.h>

#define NUM_LEDS 5
#define DATA_PIN 4
CRGB leds[NUM_LEDS];

const int servoNum = 4;
const int servoPins[servoNum] = {5, 6, 7, 1};
Servo servos[servoNum];

void setup() {
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
    for(int i = 0; i < NUM_LEDS; i++){
        leds[i].setRGB(3, 0, 0);
    }
    delay(500);
    FastLED.show();

    for(int i = 0; i < servoNum; i++){
        pinMode(servoPins[i], OUTPUT);
        servos[i].attach(servoPins[i], 853, 2090);
        servos[i].write(servoPins[i], 90);
    }
}

void f(int index, int degree){
    Serial.print("Servo is at ");
    Serial.print(degree);
    Serial.println(" degrees");
    servos[index].write(servoPins[index], degree);
    delay(1000);
}

void loop(){
    int delta = 60;
    for(int i = 0; i < servoNum; i++) f(i, 0);
    for(int i = 0; i < servoNum; i++) f(i, 90);
    for(int i = 0; i < servoNum; i++) f(i, 180);
    for(int i = 0; i < servoNum; i++) f(i, 90);
}