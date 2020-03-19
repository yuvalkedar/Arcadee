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
#define GUARD_LIMIT_SWITCH_PIN   (3)
#define SHOOTING_BTN_PIN (4)  // Right pin in the RPi (GPIO18)
#define SOLENOID_PIN (5)    // controls a relay
#define GUARD_DIR_PIN     (6)
#define GUARD_STEP_PIN    (7)
#define YAW_DIR_PIN     (8)
#define YAW_STEP_PIN    (9)
#define START_BTN_PIN (10)        // coin switch pin in the RPi (GPIO25)
#define SOLENOID_LIMIT_SWITCH_PIN   (11)
#define LIMIT_SWITCH_2_PIN (12)   // limit switch r/l pin in the RPi (GPIO20)
#define LIMIT_SWITCH_1_PIN (13)   // limit switch f/b pin in the RPi (GPIO16)

#define MOTOR_STEPS (200)
#define RPM (120)
#define MICROSTEPS (1)
#define YAW_INCREMENT_MULTIPLY  (2)
#define GUARD_INCREMENT_MULTIPLY  (2)
#define RESET_DELAY_MS (1500)
#define YAW_UPDATE_MS (40)
#define YAW_STEPPER_MIN (0)
#define YAW_STEPPER_MAX (30)
#define GUARD_STEPPER_MIN (1)
#define GUARD_STEPPER_MAX (855)
#define GUARD_UPDATE_MS (10)
#define SOLENOID_ON_TIME    (500)

Button start_btn(START_BTN_PIN);
Button aiming_btn(YAW_BTN_PIN);
Button shooting_btn(SHOOTING_BTN_PIN);

Timer reset_timer;     //resets canon after shooting a ball
Timer yaw_update_timer;    //updates the canon's position
Timer guard_update_timer;    //updates the guard's position

BasicStepperDriver yaw_stepper(MOTOR_STEPS, YAW_DIR_PIN, YAW_STEP_PIN);
BasicStepperDriver guard_stepper(MOTOR_STEPS, GUARD_DIR_PIN, GUARD_STEP_PIN);

volatile int yaw_position;
volatile int guard_position;
int yaw_position_increment = 1;   //WARNNING: the var's size might be too small.
int guard_position_increment = 1;

void limit_switches(bool state) {  //controls home sensors
    digitalWrite(LIMIT_SWITCH_1_PIN, state);
    digitalWrite(LIMIT_SWITCH_2_PIN, state);
}


//TODO: make it work without delay
void reset_stepper_position(uint8_t _dir_pin, uint8_t _step_pin, uint8_t _limit_sw) {  //go left until the limit switch is pressed
    while (!digitalRead(_limit_sw)) {
        digitalWrite(_dir_pin, HIGH);  // (HIGH = CCW / LOW = CW)
        digitalWrite(_step_pin, HIGH);
        delay(5);  // Delay to slow down speed of Stepper
        digitalWrite(_step_pin, LOW);
        delay(5);
    }
}

void game_start() {  // resets all parameters
    limit_switches(0);
    yaw_position = YAW_STEPPER_MIN;
    reset_stepper_position(YAW_DIR_PIN, YAW_STEP_PIN, SOLENOID_LIMIT_SWITCH_PIN);  // restart yaw position
}

void reset_cb() {
    limit_switches(0);
    // digitalWrite(LAUNCHER_MOTOR_PIN, HIGH);
    yaw_position = YAW_STEPPER_MIN;
    // reset_nerf_position();  // restart yaw position

}

void guard_movement() {
    guard_position += guard_position_increment;
    guard_stepper.move(guard_position_increment*GUARD_INCREMENT_MULTIPLY);
    if (guard_position <= GUARD_STEPPER_MIN || guard_position - 1 >= GUARD_STEPPER_MAX) guard_position_increment = -guard_position_increment;
    Serial.println(guard_position);
}

//TODO: fix it
void yaw_update() {
    /*
    stepper.rotate(-steps);
    yaw_position++;
    if (yaw_position <= YAW_MIN || yaw_position - 1 >= YAW_MAX) {
        steps = 0;
    } else
        steps = STEPS;
    Serial.print("count: ");
    Serial.println(yaw_position);
*/



    yaw_position -= yaw_position_increment;
    yaw_stepper.move(yaw_position_increment*YAW_INCREMENT_MULTIPLY);
    if (yaw_position <= YAW_STEPPER_MIN || yaw_position - 1 >= YAW_STEPPER_MAX) yaw_position_increment = -yaw_position_increment;
    Serial.println(yaw_position);
    // delay(70);
}

void toggle_solenoid() {
    static unsigned long cur_time = millis();
    digitalWrite(SOLENOID_PIN, HIGH);
    if (millis() - cur_time > SOLENOID_ON_TIME) digitalWrite(SOLENOID_PIN, LOW);
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

    guard_update_timer.setCallback(guard_movement);
    guard_update_timer.setInterval(GUARD_UPDATE_MS);

    pinMode(SHOOTING_BTN_PIN, INPUT);
    pinMode(YAW_BTN_PIN, INPUT);
    pinMode(START_BTN_PIN, INPUT);
    pinMode(GUARD_LIMIT_SWITCH_PIN, INPUT_PULLUP);

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
    guard_position = GUARD_STEPPER_MAX;

    reset_stepper_position(GUARD_DIR_PIN, GUARD_STEP_PIN, GUARD_LIMIT_SWITCH_PIN);  // restart guard position
    guard_update_timer.start();
}

void loop() {
    // delay(10);
    // Serial.println(digitalRead(GUARD_LIMIT_SWITCH_PIN));
    /*
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
    }

    if (shooting_btn.released()) {
        toggle_solenoid();
        reset_timer.start();
    }

*/
    TimerManager::instance().update();
}
