/*
	Author: Yuval Kedar - KD Technology
	Instagram: https://www.instagram.com/kd_technology/
	Date: Nov 19
	Dev board: Arduino Uno
	
	Nerf-Gun controller for Gigantic - Clawee.
	
	Arduino Uno communicates with RPi.
*/

#include <Adafruit_NeoPixel.h>
#include <Button.h>  // https://github.com/madleech/Button
#include <timer.h>  // https://github.com/brunocalou/Timer
#include <timerManager.h>
#include "BasicStepperDriver.h" // https://github.com/laurb9/StepperDriver

#define DEBUG

#define R_LIMIT_SWITCH_PIN    (8)
#define L_LIMIT_SWITCH_PIN    (7)
#define STEPS_PIN (3)
#define DIR_PIN (4)
#define BTM_LIMIT_SWITCH_PIN (2)  // rocket's home position
#define MOTOR_A (5)
#define MOTOR_B (6)
#define LED_DATA_PIN (9)
#define WINNING_SENSOR_PIN (13)   // winning switch pin in the RPi (GPIO12)
#define START_GAME_PIN (11)       // coin switch pin in the RPi (GPIO25)
#define LIMIT_SWITCH_2_PIN (12)   // limit switch r/l pin in the RPi (GPIO20)
#define LIMIT_SWITCH_1_PIN (10)  // limit switch f/b pin in the RPi (GPIO16)
#define LDR_1_PIN (A0)
#define LDR_2_PIN (A1)
#define LDR_3_PIN (A2)
#define LDR_4_PIN (A3)

#define RESET_MS (2000)
#define NUM_LEDS (87)
#define LED_BRIGHTNESS (50)
#define WINNING_FX_TIME (2000)  //NOTICE    : make sure the number isn't too big. User might start a new game before the effect ends.
#define LDR_1_LIMIT (200)
#define LDR_2_LIMIT (200)
#define LDR_3_LIMIT (150)
#define LDR_4_LIMIT (170)
#define MOTOR_STEPS (200)  // Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define RPM (120)
#define MICROSTEPS (1)
#define DEG_PER_LEVEL (360)
#define LEVEL_0_1_DEG (DEG_PER_LEVEL)
#define LEVEL_1_2_DEG (1.5 * DEG_PER_LEVEL)
#define LEVEL_2_3_DEG (1.5 * DEG_PER_LEVEL)
#define LEVEL_3_4_DEG (1.5 * DEG_PER_LEVEL)

Adafruit_NeoPixel strip(NUM_LEDS, LED_DATA_PIN, NEO_GRB + NEO_KHZ800);
Button coin_btn(START_GAME_PIN);
BasicStepperDriver rocket(MOTOR_STEPS, DIR_PIN, STEPS_PIN);
Timer reset_timer;

bool status = 0;
bool ls_state = 0;
int8_t score = 0;
uint8_t last_score = 0;
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

void sweep_motor() {
    if (digitalRead(L_LIMIT_SWITCH_PIN) && !digitalRead(R_LIMIT_SWITCH_PIN)) ls_state = 0;
    if (!digitalRead(L_LIMIT_SWITCH_PIN) && digitalRead(R_LIMIT_SWITCH_PIN)) ls_state = 1;

    // Serial.print(digitalRead(L_LIMIT_SWITCH_PIN));
    // Serial.print(" ");
    // Serial.print(digitalRead(R_LIMIT_SWITCH_PIN));
    // Serial.print(" ");
    // Serial.println(ls_state);
    
    if (ls_state) {  //move CW
        digitalWrite(MOTOR_A , LOW);
        digitalWrite(MOTOR_B , HIGH);
    }
    else if (!ls_state) {    //move CCW
        digitalWrite(MOTOR_A , HIGH);
        digitalWrite(MOTOR_B , LOW);
    }
}

void reset_rocket_position() {  //go down until the limit switch is pressed
  while (digitalRead(BTM_LIMIT_SWITCH_PIN)) {
    digitalWrite(DIR_PIN, LOW);      // (HIGH = anti-clockwise / LOW = clockwise)
    digitalWrite(STEPS_PIN, HIGH);
    delay(5);                       // Delay to slow down speed of Stepper
    digitalWrite(STEPS_PIN, LOW);
    delay(5);
  }
}

void colorWipe(uint32_t c, uint8_t wait) {
    for (uint8_t l = 0; l < 5; l++) {
        for (uint16_t i=0; i<strip.numPixels(); i++) {
            strip.setPixelColor(i, c);
            strip.show();
            delay_millis(wait);
        }
        strip.clear();
        strip.show();
    }
}

void winning() {    // winning effect
    rocket.rotate(LEVEL_3_4_DEG);

    colorWipe(strip.Color(255, 0, 0), 10); // Red
    strip.clear();
    strip.show();
}

void reset_game() {
    Serial.println("RESTART GAME");
    if (score != 0) rocket.rotate(-(LEVEL_3_4_DEG + LEVEL_2_3_DEG + LEVEL_1_2_DEG + LEVEL_0_1_DEG));
    status = 0;
    score = 0;
    strip.clear();
    strip.show();
    digitalWrite(WINNING_SENSOR_PIN, LOW);
    last_score = 4;
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

    //TODO: add colorWipe() by score level
    switch (score) {
        case 0:
            digitalWrite(WINNING_SENSOR_PIN, LOW);
            if (last_score == 1) rocket.rotate(-LEVEL_0_1_DEG);
            last_score = 0;
            break;
        case 1:
            digitalWrite(WINNING_SENSOR_PIN, LOW);
            if (last_score == 0) rocket.rotate(LEVEL_0_1_DEG);  // level up
            if (last_score == 2) rocket.rotate(-LEVEL_1_2_DEG);  // level down
            last_score = 1;
            break;
        case 2:
            digitalWrite(WINNING_SENSOR_PIN, LOW);
            if (last_score == 1) rocket.rotate(LEVEL_1_2_DEG);  // level up
            if (last_score == 3) rocket.rotate(-LEVEL_2_3_DEG);  // level down
            last_score = 2;
            break;
        case 3:
            digitalWrite(WINNING_SENSOR_PIN, LOW);
            if (last_score == 2) rocket.rotate(LEVEL_2_3_DEG);  // level up
            if (last_score == 4) rocket.rotate(-1.5 * DEG_PER_LEVEL);  // level down
            last_score = 3;
            break;
        case 4:
            // winning_check();
            if (!status) {  //status var is to make sure what inside will be called only once.
                status++;
                winning();
                reset_timer.start();
            }
            // last_score = 4;     //NOTICE: I might have to change this to 0 or it doesn't matter
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

    reset_timer.setCallback(reset_game);
    reset_timer.setTimeout(RESET_MS);

    Serial.println(F(
        "____________________________________\n"
        "\n"
        "  S P A C E S H I P  M a c h i n e  \n"
        "____________________________________\n"
        "\n"
        "Made by KD Technology\n"
        "\n"));

    pinMode(STEPS_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    pinMode(MOTOR_A, OUTPUT);
    pinMode(MOTOR_B, OUTPUT);
    pinMode(BTM_LIMIT_SWITCH_PIN, INPUT_PULLUP);
    pinMode(R_LIMIT_SWITCH_PIN, INPUT_PULLUP);
    pinMode(L_LIMIT_SWITCH_PIN, INPUT_PULLUP);
    pinMode(LDR_1_PIN, INPUT);
    pinMode(LDR_2_PIN, INPUT);
    pinMode(LDR_3_PIN, INPUT);
    pinMode(LDR_4_PIN, INPUT);
    pinMode(LIMIT_SWITCH_1_PIN, INPUT);  // When pressed = 0
    pinMode(LIMIT_SWITCH_2_PIN, INPUT);  // When depressed = 1

    strip.begin();
    strip.setBrightness(LED_BRIGHTNESS);
    strip.show();  // Turn OFF all pixels

    // reset_game();
    // reset_rocket_position();
}

void loop() {
    // check_for_game();
    // update_score();
    sweep_motor();
}