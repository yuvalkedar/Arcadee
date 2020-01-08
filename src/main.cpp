/*
	Author: Yuval Kedar - KD Technology
	Instagram: https://www.instagram.com/kd_technology/
	Date: Nov 19
	Dev board: Arduino Uno
	
	Nerf-Gun controller for Gigantic - Clawee.
	
	Arduino Uno communicates with RPi.
*/

#include <Button.h>  // https://github.com/madleech/Button
#include <Servo.h>
#include <timer.h>  // https://github.com/brunocalou/Timer
#include <timerManager.h>

// #define DEBUG

#define LIMIT_SWITCH (5)  // rocket's home position
#define STEPS_PIN (3)
#define DIR_PIN (4)
#define STEPS_PER_LEVEL (280)

char input_char;

//TODO: add function that gets input of dir and num of steps

void reset_rocket_position() {  //go a few steps up (just to make sure) and then go down until the limit switch is pressed
    digitalWrite(DIR_PIN, LOW);
    while (digitalRead(LIMIT_SWITCH)) {
        digitalWrite(STEPS_PIN, HIGH);
        delayMicroseconds(1000);
        digitalWrite(STEPS_PIN, LOW);
        delayMicroseconds(1000);
    }
}

void setup() {
    Serial.begin(115200);

    // Declare pins as output:
    pinMode(STEPS_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    pinMode(LIMIT_SWITCH, INPUT_PULLUP);

    reset_rocket_position();

    Serial.println(F(
        "____________________________________\n"
        "\n"
        "  S P A C E S H I P  M a c h i n e  \n"
        "____________________________________\n"
        "\n"
        "Made by KD Technology\n"
        "\n"));
}

void loop() {
    // Serial.println(digitalRead(LIMIT_SWITCH));
    input_char = Serial.read();
    switch (input_char) {
        case 'u':
            digitalWrite(DIR_PIN, HIGH);

            // Spin the stepper motor 1 revolution slowly:
            for (int i = 0; i < STEPS_PER_LEVEL; i++) {
                // These four lines result in 1 step:
                digitalWrite(STEPS_PIN, HIGH);
                delayMicroseconds(2000);
                digitalWrite(STEPS_PIN, LOW);
                delayMicroseconds(2000);
            }

            break;
        case 'd':
            // Set the spinning direction counterclockwise:
            digitalWrite(DIR_PIN, LOW);

            // Spin the stepper motor 1 revolution quickly:
            for (int i = 0; i < STEPS_PER_LEVEL; i++) {
                // These four lines result in 1 step:
                digitalWrite(STEPS_PIN, HIGH);
                delayMicroseconds(1000);
                digitalWrite(STEPS_PIN, LOW);
                delayMicroseconds(1000);
            }
            break;
    }
}