/*
	Author: Yuval Kedar - KD Technology
	Instagram: https://www.instagram.com/kd_technology/
	Date: Mar 20
	Dev board: Arduino Nano
	
    SOCCER MACHINE FOR GIGANTIC LTD.
*/

#include <Arduino.h>
#include "BasicStepperDriver.h"
#include <Button.h>  // https://github.com/madleech/Button
#include <timer.h>  // https://github.com/brunocalou/Timer
#include <timerManager.h>

#define YAW_BTN_PIN (2)  // Front pin in the RPi (GPIO17)
#define WINNING_SENSOR_PIN (3)    // winning switch pin in the RPi (GPIO12)
#define SHOOTING_BTN_PIN (4)  // Right pin in the RPi (GPIO18)
#define SOLENOID_PIN (5)    // controls a relay
#define GUARD_DIR_PIN     (6)
#define GUARD_STEP_PIN    (7)
#define YAW_DIR_PIN     (8)
#define YAW_STEP_PIN    (9)
#define START_BTN_PIN (10)        // coin switch pin in the RPi (GPIO25)
#define SOLENOID_LIMIT_SWITCH_PIN   (11)
#define GUARD_LIMIT_SWITCH_PIN   () //TODO: give it a pin
#define LIMIT_SWITCH_2_PIN (12)   // limit switch r/l pin in the RPi (GPIO20)
#define LIMIT_SWITCH_1_PIN (13)   // limit switch f/b pin in the RPi (GPIO16)

#define MOTOR_STEPS (200)
#define RPM (120)
#define MICROSTEPS (1)
#define INCREMENT_MULTIPLY  (2)
// #define GAME_TIME (20000)  // in milliseconds
#define RESET_DELAY_MS (1500)
#define YAW_UPDATE_MS (40)
#define YAW_STEPPER_MIN (0)
#define YAW_STEPPER_MAX (30)
#define GUARD_STEPPER_MIN (0)
#define GUARD_STEPPER_MAX (30)

Button start_btn(START_BTN_PIN);
Button aiming_btn(YAW_BTN_PIN);
Button shooting_btn(SHOOTING_BTN_PIN);

Timer reset_timer;     //resets canon after shooting a ball
Timer yaw_update_timer;    //updates the canon's position

BasicStepperDriver yaw_stepper(MOTOR_STEPS, YAW_DIR_PIN, YAW_STEP_PIN);
BasicStepperDriver guard_stepper(MOTOR_STEPS, GUARD_DIR_PIN, GUARD_STEP_PIN);

volatile int yaw_position;
int yaw_increment = 1;

void limit_switches(bool state) {  //controls home sensors
    digitalWrite(LIMIT_SWITCH_1_PIN, state);
    digitalWrite(LIMIT_SWITCH_2_PIN, state);
}

void guard_movement() {

}

//TODO: make it work without delay
void reset_stepper_position(uint8_t _dir_pin, uint8_t _step_pin, uint8_t _limit_sw) {  //go left until the limit switch is pressed
    while (digitalRead(_limit_sw)) {
        digitalWrite(_dir_pin, HIGH);  // (HIGH = CCW / LOW = CW)
        digitalWrite(_step_pin, HIGH);
        delay(5);  // Delay to slow down speed of Stepper
        digitalWrite(_step_pin, LOW);
        delay(5);
    }
}

void winning_check(uint16_t mask) {
    if ((mask & 0b11111000000000) == 0b11111000000000) {  // btm row winning sequence
        digitalWrite(WINNING_SENSOR_PIN, HIGH);
        digitalWrite(BTM_ROW_MOTOR_PIN, LOW);  // =turn on
    } else {
        // digitalWrite(WINNING_SENSOR_PIN, LOW);
        if (!digitalRead(BTM_ROW_SENSOR_PIN)) digitalWrite(BTM_ROW_MOTOR_PIN, HIGH);
    }
    if ((mask & 0b00000111110000) == 0b00000111110000) {  // mid row winning sequence
        digitalWrite(WINNING_SENSOR_PIN, HIGH);
        digitalWrite(MID_ROW_MOTOR_PIN, LOW);  // =turn on
    } else {
        // digitalWrite(WINNING_SENSOR_PIN, LOW);
        if (!digitalRead(MID_ROW_SENSOR_PIN)) digitalWrite(MID_ROW_MOTOR_PIN, HIGH);
    }
    if ((mask & 0b00000000001111) == 0b00000000001111) {  // top row winning sequence
        digitalWrite(WINNING_SENSOR_PIN, HIGH);
        digitalWrite(TOP_ROW_MOTOR_PIN, LOW);  // =turn on
    } else {
        // digitalWrite(WINNING_SENSOR_PIN, LOW);
        if (!digitalRead(TOP_ROW_SENSOR_PIN)) digitalWrite(TOP_ROW_MOTOR_PIN, HIGH);
    }
}

void game_start() {  // resets all parameters
    digitalWrite(WINNING_SENSOR_PIN, LOW);
    limit_switches(0);
    yaw_position = YAW_MIN;
    reset_stepper_position(YAW_DIR_PIN, YAW_STEP_PIN, SOLENOID_LIMIT_SWITCH_PIN);  // restart yaw position
}

void reset_cb() {
    limit_switches(0);
    // digitalWrite(LAUNCHER_MOTOR_PIN, HIGH);
    stop_belt();

    // pitch_position = PITCH_RESTART_POSITION;
    // pitch.write(pitch_position);  // restart pitch position

    yaw_position = YAW_MIN;
    // reset_nerf_position();  // restart yaw position

    // digitalWrite(WINNING_SENSOR_PIN, LOW);
}

void yaw_update() {
    stepper.rotate(-steps);
    yaw_position++;
    if (yaw_position <= YAW_MIN || yaw_position - 1 >= YAW_MAX) {
        steps = 0;
    } else
        steps = STEPS;
    Serial.print("count: ");
    Serial.println(yaw_position);




        yaw_position -= yaw_increment;
    yaw_stepper.move(yaw_increment*INCREMENT_MULTIPLY);
    if (yaw_position <= YAW_STEPPER_MIN || yaw_position - 1 >= YAW_STEPPER_MAX) yaw_increment = -yaw_increment;
    Serial.println(yaw_position);
    delay(70);
}

void setup() {
    Serial.begin(115200);
    start_btn.begin();
    aiming_btn.begin();
    shooting_btn.begin();

    yaw_stepper.begin(RPM, MICROSTEPS);
    guard_stepper.begin(RPM, MICROSTEPS);

    reset_timer.setCallback(reset_cb);
    reset_timer.setTimeout(RESET_DELAY_MS);

    yaw_update_timer.setCallback(yaw_update);
    yaw_update_timer.setInterval(YAW_UPDATE_MS);

    pinMode(SHOOTING_BTN_PIN, INPUT);
    pinMode(YAW_BTN_PIN, INPUT);
    pinMode(START_BTN_PIN, INPUT);
    // digitalWrite(WINNING_SENSOR_PIN, LOW);

    pinMode(LIMIT_SWITCH_1_PIN, OUTPUT);
    pinMode(LIMIT_SWITCH_2_PIN, OUTPUT);
    limit_switches(0);

    Serial.println(F(
        "_________________________________\n"
        "\n"
        "   S o c c e r   M a c h i n e   \n"
        "_________________________________\n"
        "\n"
        "Made by KD Technology\n"
        "\n"));

    yaw_position = YAW_STEPPER_MAX;
    reset_stepper_position(GUARD_DIR_PIN, GUARD_STEP_PIN, GUARD_LIMIT_SWITCH_PIN);  // restart guard position
}

void loop() {
    if (start_btn.pressed()) {
        // Serial.println("game starts");
        game_start();  //based on 1000us of the coin pin
    }

    if (!digitalRead(YAW_BTN_PIN) && !yaw_update_timer.isRunning()) {
        // Serial.println("#1 btn is pressed");
        digitalWrite(LIMIT_SWITCH_2_PIN, HIGH);
        yaw_update_timer.start();
    }
    if (yaw_btn.released() && yaw_update_timer.isRunning()) yaw_update_timer.stop();

    if (!digitalRead(SHOOTING_BTN_PIN)) {
        // Serial.println("#2 btn is pressed");
        digitalWrite(LIMIT_SWITCH_1_PIN, HIGH);
        digitalWrite(LAUNCHER_MOTOR_PIN, LOW);
    }

    if (shooting_btn.released()) {
        reset_timer.start();
    }

    winning_check(get_clowns_state());
    TimerManager::instance().update();
}
