/*
	Author: Yuval Kedar - KD Technology
	Instagram: https://www.instagram.com/kd_technology/
	Date: Jan 2020
	Dev board: Arduino nano
	
	Spaceship Machine (progress) for Gigantic - Clawee.
	
	Arduino Uno communicates with RPi.
*/

#include <Adafruit_NeoPixel.h>
#include <Button.h>  // https://github.com/madleech/Button
#include <timer.h>   // https://github.com/brunocalou/Timer
#include <timerManager.h>
#include "BasicStepperDriver.h"  // https://github.com/laurb9/StepperDriver

// #define DEBUG

#define R_LIMIT_SWITCH_PIN (8)
#define L_LIMIT_SWITCH_PIN (7)
#define STEPS_PIN (3)
#define DIR_PIN (4)
#define BTM_LIMIT_SWITCH_PIN (2)  // rocket's home position
#define MOTOR_A (5)
#define MOTOR_B (6)
#define LED_DATA_PIN (9)
#define WINNING_SENSOR_PIN (13)  // winning switch pin in the RPi (GPIO12)
#define LDR_1_PIN (A0)
#define LDR_2_PIN (A1)
#define LDR_3_PIN (A2)
#define LDR_4_PIN (A3)

#define RESET_MS (2000)
#define NUM_LEDS (87)
#define LED_BRIGHTNESS (50)
#define WINNING_FX_TIME (2000)  //NOTICE: make sure the number isn't too big. User might start a new game before the effect ends.
#define LDR_1_LIMIT (120)
#define LDR_2_LIMIT (120)
#define LDR_3_LIMIT (120)
#define LDR_4_LIMIT (120)
#define MOTOR_STEPS (200)  // Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define RPM (120)
#define MICROSTEPS (1)
#define DEG_PER_LEVEL (600)
#define LEVEL_0_1_DEG (DEG_PER_LEVEL + 100)
#define LEVEL_1_2_DEG (910)
#define LEVEL_2_3_DEG (950)
#define LEVEL_3_4_DEG (950)
#define DEBOUNCE_MS (100)

Adafruit_NeoPixel strip(NUM_LEDS, LED_DATA_PIN, NEO_GRB + NEO_KHZ800);
BasicStepperDriver rocket(MOTOR_STEPS, DIR_PIN, STEPS_PIN);
Timer reset_timer;

bool status = 0;
bool ls_state = 0;
int8_t score = 0;
uint8_t last_score = 0;
int increment_steps = 1;
uint32_t cur_time;

void delay_millis(uint32_t ms) {
    uint32_t start_ms = millis();
    while (millis() - start_ms < ms)
        ;
}

void sweep_motor() {
    if (digitalRead(L_LIMIT_SWITCH_PIN) && !digitalRead(R_LIMIT_SWITCH_PIN)) ls_state = 0;
    if (!digitalRead(L_LIMIT_SWITCH_PIN) && digitalRead(R_LIMIT_SWITCH_PIN)) ls_state = 1;

    // Serial.print(digitalRead(L_LIMIT_SWITCH_PIN));
    // Serial.print(" ");
    // Serial.print(digitalRead(R_LIMIT_SWITCH_PIN));
    // Serial.print(" ");
    // Serial.println(ls_state);

    if (ls_state) {  //move CW
        digitalWrite(MOTOR_A, LOW);
        digitalWrite(MOTOR_B, HIGH);
    } else if (!ls_state) {  //move CCW
        digitalWrite(MOTOR_A, HIGH);
        digitalWrite(MOTOR_B, LOW);
    }
}

void reset_rocket_position() {  //go down until the limit switch is pressed
    while (digitalRead(BTM_LIMIT_SWITCH_PIN)) {
        digitalWrite(DIR_PIN, HIGH);  // (HIGH = anti-clockwise / LOW = clockwise)
        digitalWrite(STEPS_PIN, HIGH);
        delay(5);  // Delay to slow down speed of Stepper
        digitalWrite(STEPS_PIN, LOW);
        delay(5);
    }
}

void colorWipe(uint32_t c, uint8_t wait) {
    for (uint8_t l = 0; l < 5; l++) {
        for (uint16_t i = 0; i < strip.numPixels(); i++) {
            strip.setPixelColor(i, c);
            strip.show();
            delay_millis(wait);
        }
        strip.clear();
        strip.show();
    }
}

void winning() {  // winning effect
    rocket.rotate(-LEVEL_3_4_DEG);

    colorWipe(strip.Color(255, 0, 0), 10);  // Red
    strip.clear();
    strip.show();
}

void reset_game() {
    // Serial.println("RESTART GAME");
    if (score != 0) rocket.rotate((LEVEL_3_4_DEG + LEVEL_2_3_DEG + LEVEL_1_2_DEG + LEVEL_0_1_DEG));
    status = 0;
    score = 0;
    strip.clear();
    strip.show();
    digitalWrite(WINNING_SENSOR_PIN, LOW);
}

void winning_check() {
    if (score == 4) {
        digitalWrite(WINNING_SENSOR_PIN, HIGH);
        // Serial.println("YOU WON");
    } else
        digitalWrite(WINNING_SENSOR_PIN, LOW);
}

void update_score() {
    uint16_t ldr_1_current_read = analogRead(LDR_1_PIN);
    uint16_t ldr_2_current_read = analogRead(LDR_2_PIN);
    uint16_t ldr_3_current_read = analogRead(LDR_3_PIN);
    uint16_t ldr_4_current_read = analogRead(LDR_4_PIN);

    if (millis() - cur_time >= DEBOUNCE_MS) {
        if ((ldr_1_current_read > LDR_1_LIMIT) || (ldr_3_current_read > LDR_3_LIMIT)) {
            score++;
            if (score >= 4) {
                score = 4;
            }
        }

        if ((ldr_2_current_read > LDR_2_LIMIT) || (ldr_4_current_read > LDR_4_LIMIT)) {
            score--;
            if (score <= 0) {
                score = 0;
            }
            // delay_millis(2000);
        }
        #ifdef DEBUG
            Serial.println();
            Serial.print(ldr_1_current_read);
            Serial.print(" ");
            Serial.print(ldr_2_current_read);
            Serial.print(" ");
            Serial.print(ldr_3_current_read);
            Serial.print(" ");
            Serial.print(ldr_4_current_read);
            Serial.print(" ");
            Serial.print(score);
            Serial.print(" ");
            Serial.println(last_score);
        #endif
        cur_time = millis();
    }

    switch (score) {
        case 0:
            digitalWrite(WINNING_SENSOR_PIN, LOW);
            if (last_score == 1) rocket.rotate(LEVEL_0_1_DEG);
            last_score = 0;
            break;
        case 1:
            digitalWrite(WINNING_SENSOR_PIN, LOW);
            if (last_score == 0) rocket.rotate(-LEVEL_0_1_DEG);   // level up
            if (last_score == 2) rocket.rotate(LEVEL_1_2_DEG);  // level down
            last_score = 1;
            break;
        case 2:
            digitalWrite(WINNING_SENSOR_PIN, LOW);
            if (last_score == 1) rocket.rotate(-LEVEL_1_2_DEG);   // level up
            if (last_score == 3) rocket.rotate(LEVEL_2_3_DEG);  // level down
            last_score = 2;
            break;
        case 3:
            digitalWrite(WINNING_SENSOR_PIN, LOW);
            if (last_score == 2) rocket.rotate(-LEVEL_2_3_DEG);  // level up
            last_score = 3;
            break;
        case 4:
            if (!status) {  //status var is to make sure what inside will be called only once.
                winning_check();
                status++;
                winning();
                reset_timer.start();
            }
            break;
    }
}

void setup() {
    Serial.begin(115200);
    rocket.begin(RPM, MICROSTEPS);

    reset_timer.setCallback(reset_game);
    reset_timer.setTimeout(RESET_MS);

    Serial.println(F(
        "____________________________________\n"
        "\n"
        " S P A C E S H I P    M a c h i n e \n"
        "____________________________________\n"
        "\n"
        "       ~Made by KD Technology~      \n"
        "\n"));

    pinMode(STEPS_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    pinMode(MOTOR_A, OUTPUT);
    pinMode(MOTOR_B, OUTPUT);
    pinMode(WINNING_SENSOR_PIN, OUTPUT);
    pinMode(BTM_LIMIT_SWITCH_PIN, INPUT_PULLUP);
    pinMode(R_LIMIT_SWITCH_PIN, INPUT_PULLUP);
    pinMode(L_LIMIT_SWITCH_PIN, INPUT_PULLUP);
    pinMode(LDR_1_PIN, INPUT);
    pinMode(LDR_2_PIN, INPUT);
    pinMode(LDR_3_PIN, INPUT);
    pinMode(LDR_4_PIN, INPUT);

    strip.begin();
    strip.setBrightness(LED_BRIGHTNESS);
    strip.clear();
    strip.show();  // Turn OFF all pixels

    digitalWrite(WINNING_SENSOR_PIN, LOW);
    status = 0;
    score = 0;
    last_score = 0;
    reset_rocket_position();
    cur_time = millis();
}

void loop() {
    update_score();
    sweep_motor();

    TimerManager::instance().update();
}