/*********
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com/esp32-servo-motor-web-server-arduino-ide/
  Based on the ESP32Servo Sweep Example
*********/

#include <ESP32Servo.h>

static const int servoPin = 13;
int pos = 0;

Servo servo1;

void setup() {

  Serial.begin(115200);
  servo1.attach(servoPin);
  servo1.write(pos);
}

void loop() {
  for (; pos <= 180; pos++){
    servo1.write(pos);
    delay(100);
  }
  pos = 0;
}