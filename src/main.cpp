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
#include "BasicStepperDriver.h"

// #define DEBUG

#define STEPS_PIN (3)
#define DIR_PIN (4)
#define LIMIT_SWITCH_PIN (5)  // rocket's home position
#define SERVO_PIN (6)
#define WINNING_SENSOR_PIN (7)   // winning switch pin in the RPi (GPIO12)
#define START_GAME_PIN (8)       // coin switch pin in the RPi (GPIO25)
#define LIMIT_SWITCH_2_PIN (9)   // limit switch r/l pin in the RPi (GPIO20)
#define LIMIT_SWITCH_1_PIN (10)  // limit switch f/b pin in the RPi (GPIO16)
#define LDR_1_PIN (A0)
#define LDR_2_PIN (A1)
#define LDR_3_PIN (A2)
#define LDR_4_PIN (A3)

#define STEPS_PER_LEVEL (280)
#define SERVO_UPDATE_MS (100)
#define SERVO_MIN_POSITION (0)
#define SERVO_MAX_POSITION (180)
#define LDR_1_LIMIT (200)
#define LDR_2_LIMIT (200)
#define LDR_3_LIMIT (150)
#define LDR_4_LIMIT (170)
#define MOTOR_STEPS (200)  // Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define RPM (120)
#define MICROSTEPS (1)

Button coin_btn(START_GAME_PIN);
BasicStepperDriver rocket(MOTOR_STEPS, DIR_PIN, STEPS_PIN);
Servo wheel_servo;
Timer servo_update_timer;

char input_char;
int8_t score = 0;
uint8_t last_score = 0;
uint8_t servo_position = SERVO_MAX_POSITION;
int increment_steps = 1;
uint16_t ldr_1_current_read = 0;
uint16_t ldr_1_prev_read = 0;
uint16_t ldr_2_current_read = 0;
uint16_t ldr_2_prev_read = 0;
uint16_t ldr_3_current_read = 0;
uint16_t ldr_3_prev_read = 0;
uint16_t ldr_4_current_read = 0;
uint16_t ldr_4_prev_read = 0;

void delay_millis(uint32_t ms) {
    uint32_t start_ms = millis();
    while (millis() - start_ms < ms)
        ;
}

void move_rocket(uint8_t dir, uint16_t steps) {
    // (dir == 1) ? digitalWrite(DIR_PIN, HIGH) : digitalWrite(DIR_PIN, LOW);  // if dir == 1 dir is up else dir is down

    // for (int i = 0; i < steps; i++) {
    //     // These four lines result in 1 step:
    //     digitalWrite(STEPS_PIN, HIGH);
    //     delayMicroseconds(2000);
    //     digitalWrite(STEPS_PIN, LOW);
    //     delayMicroseconds(2000);
    // }
}

void reset_rocket_position() {  //go a few steps up (just to make sure) and then go down until the limit switch is pressed
    digitalWrite(DIR_PIN, LOW);
    while (digitalRead(LIMIT_SWITCH_PIN)) {
        digitalWrite(STEPS_PIN, HIGH);
        delayMicroseconds(1000);
        digitalWrite(STEPS_PIN, LOW);
        delayMicroseconds(1000);
    }
}

void servo_sweep() {
    servo_position -= increment_steps;
    wheel_servo.write(servo_position);
    if (servo_position <= SERVO_MIN_POSITION || servo_position - 1 >= SERVO_MAX_POSITION) increment_steps = -increment_steps;
    delay_millis(100);
}

void winning_check() {
    if (score == 4) {
        digitalWrite(WINNING_SENSOR_PIN, HIGH);
        // Serial.println("YOU WON");
    } else
        digitalWrite(WINNING_SENSOR_PIN, LOW);
}

void update_score() {
    ldr_1_current_read = analogRead(LDR_1_PIN);
    ldr_2_current_read = analogRead(LDR_2_PIN);
    ldr_3_current_read = analogRead(LDR_3_PIN);
    ldr_4_current_read = analogRead(LDR_4_PIN);

    if ((ldr_1_current_read > LDR_1_LIMIT && ldr_1_prev_read < LDR_1_LIMIT) || (ldr_3_current_read > LDR_3_LIMIT && ldr_3_prev_read < LDR_3_LIMIT)) {
        score++;
        if (score >= 4) {
            score = 4;
        }
    }
    ldr_1_prev_read = ldr_1_current_read;
    ldr_3_prev_read = ldr_3_current_read;

    if ((ldr_2_current_read > LDR_2_LIMIT && ldr_2_prev_read < LDR_2_LIMIT) || (ldr_4_current_read > LDR_4_LIMIT && ldr_4_prev_read < LDR_4_LIMIT)) {
        score--;
        if (score <= 0) {
            score = 0;
        }
    }
    ldr_2_prev_read = ldr_2_current_read;
    ldr_4_prev_read = ldr_4_current_read;

    // Serial.println();
    // Serial.print(ldr_1_current_read);
    // Serial.print(" ");
    // Serial.print(ldr_2_current_read);
    // Serial.print(" ");
    // Serial.print(ldr_3_current_read);
    // Serial.print(" ");
    // Serial.print(ldr_4_current_read);
    // Serial.print(" ");
    // Serial.println(score);

    switch (score) {
        case 0:
            if (last_score == 1) move_rocket(0, STEPS_PER_LEVEL);
            last_score = 0;
            digitalWrite(WINNING_SENSOR_PIN, LOW);
            break;
        case 1:
            if (last_score == 0) move_rocket(1, STEPS_PER_LEVEL);  // level up
            if (last_score == 2) move_rocket(0, STEPS_PER_LEVEL);  // level down
            last_score = 1;
            digitalWrite(WINNING_SENSOR_PIN, LOW);
            break;
        case 2:
            if (last_score == 1) move_rocket(1, STEPS_PER_LEVEL);  // level up
            if (last_score == 3) move_rocket(0, STEPS_PER_LEVEL);  // level down
            last_score = 2;
            digitalWrite(WINNING_SENSOR_PIN, LOW);
            break;
        case 3:
            if (last_score == 2) move_rocket(1, STEPS_PER_LEVEL);  // level up
            if (last_score == 4) move_rocket(0, STEPS_PER_LEVEL);  // level down
            last_score = 3;
            digitalWrite(WINNING_SENSOR_PIN, LOW);
            break;
        case 4:
            winning_check();
            // reset_timer.start();
            // winning();
            break;
    }
}

void check_for_game() {
    if (coin_btn.pressed()) {
        digitalWrite(WINNING_SENSOR_PIN, LOW);
    }
    /*
    limits_state = digitalRead(LIMIT_SWITCH_1_PIN) && digitalRead(LIMIT_SWITCH_2_PIN);

    //NOTICE: the arduino doesn't know a game starts if someone plays manually. For that I need to add a condition down here.
    if (limits_state) {  // claw moved and GAME ON
        // Serial.println("GAME ON!!!");
        prev_limits_state = true;
    }

    if (!digitalRead(LIMIT_SWITCH_1_PIN) && !digitalRead(LIMIT_SWITCH_2_PIN) && prev_limits_state) {  // GAME OVER...
        strip.clear();
        strip.show();
        prev_limits_state = false;
    }
    */
}

void setup() {
    Serial.begin(115200);
    coin_btn.begin();
    rocket.begin(RPM, MICROSTEPS);

    Serial.println(F(
        "____________________________________\n"
        "\n"
        "  S P A C E S H I P  M a c h i n e  \n"
        "____________________________________\n"
        "\n"
        "Made by KD Technology\n"
        "\n"));

    wheel_servo.attach(SERVO_PIN);
    wheel_servo.write(SERVO_MIN_POSITION);

    servo_update_timer.setCallback(servo_sweep);
    servo_update_timer.setInterval(SERVO_UPDATE_MS);

    // Declare pins as output:
    pinMode(STEPS_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP);
    pinMode(LDR_1_PIN, INPUT);
    pinMode(LDR_2_PIN, INPUT);
    pinMode(LDR_3_PIN, INPUT);
    pinMode(LDR_4_PIN, INPUT);
    pinMode(LIMIT_SWITCH_1_PIN, INPUT);  // When pressed = 0
    pinMode(LIMIT_SWITCH_2_PIN, INPUT);  // When depressed = 1

    // reset_rocket_position();
    // servo_update_timer.start();
}

void loop() {
#ifdef DEBUG
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
    update_score();

#else
    // servo_sweep();
    // check_for_game();
    // update_score();
    // move_rocket(0, STEPS_PER_LEVEL);
    // TimerManager::instance().update();

    rocket.rotate(30);

    // Serial.println();
    // Serial.print(ldr_1_current_read);
    // Serial.print(" ");
    // Serial.print(ldr_2_current_read);
    // Serial.print(" ");
    // Serial.print(ldr_3_current_read);
    // Serial.print(" ");
    // Serial.print(ldr_4_current_read);
    // Serial.print(" ");
    // Serial.println(score);
    // delay(10);
#endif
}