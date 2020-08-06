#include <Arduino.h>
#include <Servo.h>
#include <timer.h>  // https://github.com/brunocalou/Timer
#include <timerManager.h>

// #define DEBUG

#define BTN_PIN (4)
#define SERVO_PIN (3)
#define WINNING_SENSOR_PIN (7)

#define SERVO_MIN (20)
#define SERVO_MAX (110)
#define RESET_DELAY_MS  (2000)

Timer reset_timer;
Servo door;

void reset_cb() {
    door.write(SERVO_MAX);
    digitalWrite(WINNING_SENSOR_PIN, HIGH);
    Serial.println("won");
}

void setup() {
    Serial.begin(115200);
    pinMode(BTN_PIN, INPUT_PULLUP);  
    pinMode(SERVO_PIN, OUTPUT);
    pinMode(WINNING_SENSOR_PIN, OUTPUT);

    digitalWrite(WINNING_SENSOR_PIN, HIGH);
    door.attach(SERVO_PIN);
    door.write(SERVO_MAX);

    reset_timer.setCallback(reset_cb);
    reset_timer.setTimeout(RESET_DELAY_MS);

    Serial.println(F(
    "_____________________________________\n"
    "\n"
    "     W E I G H T   M a c h i n e     \n"
    "_____________________________________\n"
    "\n"
    "         Made by KD Technology      \n"
    "\n"));
}

void loop() {
    if (!digitalRead(BTN_PIN)) {
        Serial.println("pressed");
        door.write(SERVO_MIN);
        digitalWrite(WINNING_SENSOR_PIN, LOW);
        reset_timer.start();
    }
    // else {
                // digitalWrite(WINNING_SENSOR_PIN, LOW);
    // }

    TimerManager::instance().update();
}