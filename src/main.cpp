/*
	Author: Yuval Kedar - KD Technology
	Instagram: https://www.instagram.com/kd_technology/
	Date: Mar 20
	Dev board: Arduino Nano
	
    SOCCER MACHINE FOR GIGANTIC LTD.
*/

// #include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include "BasicStepperDriver.h"
// #include <Button.h>  // https://github.com/madleech/Button
// #include <timer.h>  // https://github.com/brunocalou/Timer
// #include <timerManager.h>

// #define LAUNCHER_BTN_PIN (4)  // Right pin in the RPi (GPIO18)
// #define LAUNCHER_PIN (3)
// #define AIMING_BTN_PIN (2)  // Front pin in the RPi (GPIO17)
// #define AIMING_SERVO_PIN (5)
// #define MAGAZINE_SERVO_PIN (6)
// #define WINNING_SENSOR_PIN (7)    // winning switch pin in the RPi (GPIO12)
// #define START_GAME_PIN (8)        // coin switch pin in the RPi (GPIO25)
// #define LIMIT_SWITCH_2_PIN (12)   // limit switch r/l pin in the RPi (GPIO20)
// #define LIMIT_SWITCH_1_PIN (13)   // limit switch f/b pin in the RPi (GPIO16)
// #define LED_BAR_PIN (A0)          // D14
// #define MAGAZINE_SENSOR_PIN (A1)  // D15
// #define BASKET_SENSOR_PIN (A2)    // D16
// #define MAGAZINE_BLOWER_PIN (A5)  // D19 = air blower
#define DIR_PIN     (8)
#define STEP_PIN    (9)

#define MOTOR_STEPS (200)
#define RPM (120)
#define MICROSTEPS (1)
#define INCREMENT_MULTIPLY  (10)
// #define NUM_PIXELS (8)
// #define GAME_TIME (20000)  // in milliseconds
// #define BASKET_SENSOR_LIMIT (500)
// #define MAGAZINE_SENSOR_LIMIT (350)
// #define LAUNCHER_DELAY_MS (500)
// #define RESET_DELAY_MS (1500)
// #define LED_BAR_BRIGHTNESS (250)
// #define CANON_MIN (0)
// #define CANON_MAX (180)
// #define CALIBRATION_MS (1000)
// #define CANON_STRENGTH (49)
// #define LED_BAR_UPDATE_MS (300)
// #define AIMING_UPDATE_MS (40)
#define AIMING_STEPPER_MIN (0)
#define AIMING_STEPPER_MAX (70)
// #define MAGAZINE_LOADING_POSITION (90)
// #define MAGAZINE_RESTART_POSITION (180)

// Button start_btn(START_GAME_PIN);
// Button aiming_btn(AIMING_BTN_PIN);
// Button launcher_btn(LAUNCHER_BTN_PIN);

// Timer launcher_timer;  //a "delay" after releasing the second button and before shooting (for canon motor to reach its speed)
// Timer reset_timer;     //resets canon after shooting a ball
// Timer strength_timer;  //updates the canon's strength and led_bar
// Timer aiming_timer;    //updates the canon's position

// Adafruit_NeoPixel strength_bar(NUM_PIXELS, LED_BAR_PIN, NEO_GRB + NEO_KHZ800);
BasicStepperDriver yaw_stepper(MOTOR_STEPS, DIR_PIN, STEP_PIN);

// volatile uint8_t led_bar = 0;
// volatile uint32_t led_bar_colour[NUM_PIXELS] = {0x00cc00, 0x00cc00, 0x66cc00, 0xcccc00, 0xff9900, 0xff6600, 0xff3300, 0xff0000};

volatile int yaw_position;
int yaw_increment = 1;

void setup() {
    Serial.begin(115200);
    // canon_begin();
    // start_btn.begin();
    // launcher_btn.begin();
    // aiming_btn.begin();
    // strength_bar.begin();

    yaw_stepper.begin(RPM, MICROSTEPS);

    //TODO: add attach and detach when a game starts and ends.
    // aiming.attach(AIMING_SERVO_PIN);
    // aiming.write(AIMING_SERVO_MAX);  // restart aiming servo position
    // magazine.attach(MAGAZINE_SERVO_PIN);
    // magazine.write(MAGAZINE_RESTART_POSITION);

    // launcher_timer.setCallback(launcher_cb);
    // launcher_timer.setTimeout(LAUNCHER_DELAY_MS);

    // reset_timer.setCallback(reset_cb);
    // reset_timer.setTimeout(RESET_DELAY_MS);

    // strength_timer.setCallback(canon_update);
    // strength_timer.setInterval(LED_BAR_UPDATE_MS);

    // aiming_timer.setCallback(aiming_update);
    // aiming_timer.setInterval(AIMING_UPDATE_MS);

    // pinMode(LAUNCHER_BTN_PIN, INPUT);
    // pinMode(AIMING_BTN_PIN, INPUT);
    // pinMode(START_GAME_PIN, INPUT);
    // pinMode(BASKET_SENSOR_PIN, INPUT);
    // pinMode(MAGAZINE_SENSOR_PIN, INPUT);
    // pinMode(MAGAZINE_BLOWER_PIN, OUTPUT);
    // pinMode(WINNING_SENSOR_PIN, OUTPUT);
    // digitalWrite(WINNING_SENSOR_PIN, LOW);

    // pinMode(LIMIT_SWITCH_1_PIN, OUTPUT);
    // pinMode(LIMIT_SWITCH_2_PIN, OUTPUT);
    // limit_switches(0);

    Serial.println(F(
        "_________________________________\n"
        "\n"
        "   S o c c e r   M a c h i n e   \n"
        "_________________________________\n"
        "\n"
        "Made by KD Technology\n"
        "\n"));

        yaw_position = AIMING_STEPPER_MAX;
}

void loop() {

    yaw_position -= yaw_increment;
    yaw_stepper.move(yaw_increment*INCREMENT_MULTIPLY);
    if (yaw_position <= AIMING_STEPPER_MIN || yaw_position - 1 >= AIMING_STEPPER_MAX) yaw_increment = -yaw_increment;
    Serial.println(yaw_position);
    delay(10);
}
