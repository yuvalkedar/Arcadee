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

#define LOAD_BALL_BTN_PIN (2)     // Right pin in the RPi (GPIO18) (LEFT & RIGHT)
#define RELEASE_BALL_BTN_PIN (3)  // Front pin in the RPi (GPIO17) (UP & DOWN)
#define WINNING_SENSOR_PIN (4)    // winning switch pin in the RPi (GPIO12)
#define LOAD_BALL_SERVO_PIN (5)
#define RELEASE_BALL_SERVO_PIN (6)
#define START_GAME_PIN (7)      // coin switch pin in the RPi (GPIO25)
#define LIMIT_SWITCH_2_PIN (8)  // limit switch r/l pin in the RPi (GPIO20)
#define LIMIT_SWITCH_1_PIN (9)  // limit switch f/b pin in the RPi (GPIO16)

#define RESET_GAME_MS (1500)
#define LOADING_RESTART_POSITION (10)
#define LOADING_POSITION (80)
#define RELEASING_RESTART_POSITION (10)
#define RELEASING_POSITION (80)

Button start_btn(START_GAME_PIN);
Button load_ball_btn(LOAD_BALL_BTN_PIN);
Button release_ball_btn(RELEASE_BALL_BTN_PIN);

Timer reset_timer;  //resets the canon after shooting a ball

Servo load_servo;
Servo release_servo;

void limit_switches(bool state) {
    digitalWrite(LIMIT_SWITCH_1_PIN, state);
    digitalWrite(LIMIT_SWITCH_2_PIN, state);
}

/*
void winning_check() {
    if (analogRead(BASKET_SENSOR_PIN) > BASKET_SENSOR_LIMIT) {
        digitalWrite(WINNING_SENSOR_PIN, HIGH);
        // Serial.println(F("Congrats son, you might be the next Kobe Bryant!"));
    } else {
        digitalWrite(WINNING_SENSOR_PIN, LOW);
        // Serial.println(F("FUCKKK"));
    }
}
*/

void game_start() {  // resets all parameters
    digitalWrite(WINNING_SENSOR_PIN, LOW);
    limit_switches(0);

    //NOTICE: might be redandent bc it's in the reset_cb
    load_servo.write(LOADING_RESTART_POSITION);
    release_servo.write(RELEASING_RESTART_POSITION);
}

void reset_cb() {
    limit_switches(0);
    digitalWrite(WINNING_SENSOR_PIN, LOW);
    load_servo.write(LOADING_RESTART_POSITION);
    release_servo.write(RELEASING_RESTART_POSITION);
}

void setup() {
    Serial.begin(115200);
    start_btn.begin();
    load_ball_btn.begin();
    release_ball_btn.begin();

    load_servo.attach(LOAD_BALL_SERVO_PIN);
    load_servo.write(LOADING_RESTART_POSITION);
    release_servo.attach(RELEASE_BALL_SERVO_PIN);
    release_servo.write(RELEASING_RESTART_POSITION);

    reset_timer.setCallback(reset_cb);
    reset_timer.setTimeout(RESET_GAME_MS);

    pinMode(LOAD_BALL_BTN_PIN, INPUT);
    pinMode(RELEASE_BALL_BTN_PIN, INPUT);
    pinMode(START_GAME_PIN, INPUT);
    pinMode(WINNING_SENSOR_PIN, OUTPUT);
    digitalWrite(WINNING_SENSOR_PIN, LOW);

    pinMode(LIMIT_SWITCH_1_PIN, OUTPUT);
    pinMode(LIMIT_SWITCH_2_PIN, OUTPUT);
    limit_switches(0);

    Serial.println(F(
        "________________________________\n"
        "\n"
        "      A l w a y s   W i n       \n"
        "         M a c h i n e          \n"
        "________________________________\n"
        "\n"
        "Made by KD Technology\n"
        "\n"));
}

void loop() {
    if (start_btn.pressed()) game_start();  //based on 1000us of the coin pin
    if (load_ball_btn.pressed() || release_ball_btn.pressed()) limit_switches(1);

    if (load_ball_btn.released()) load_servo.write(LOADING_POSITION);
    if (release_ball_btn.released()) {
        release_servo.write(RELEASING_POSITION);
        digitalWrite(WINNING_SENSOR_PIN, HIGH);
    }

    TimerManager::instance().update();
}
