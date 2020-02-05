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

/*
//NOTICE: Relays work on LOW! (their state in HIGH when pin is LOW and vice versa)
These are the pins that get affected:
TOP_ROW_MOTOR_PIN, MID_ROW_MOTOR_PIN, BTM_ROW_MOTOR_PIN,
LAUNCHER_MOTOR_PIN, BELT_MOTOR_PIN, BLOWER_MOTOR_PIN
*/

// OUTPUTS
#define PITCH_SERVO_PIN (5)
#define STEPPER_STEPS_PIN (6)
#define STEPPER_DIR_PIN (7)
#define BTM_LIMIT_SWITCH_PIN (8)
#define LIMIT_SWITCH_2_PIN (16)  // limit switch r/l pin in the RPi (GPIO20)
#define LIMIT_SWITCH_1_PIN (17)  // limit switch f/b pin in the RPi (GPIO16)
#define LAUNCHER_MOTOR_PIN (20)  // shooting motor
#define BLOWER_MOTOR_PIN (19)    // loads balls into the magazine
#define BELT_MOTOR_PIN (15)      // loads balls into the launcher
#define WINNING_SENSOR_PIN (22)  // winning switch pin in the RPi (GPIO12)
#define TOP_ROW_MOTOR_PIN (23)
#define MID_ROW_MOTOR_PIN (24)
#define BTM_ROW_MOTOR_PIN (25)
// INPUTS
#define MOTOR_STEPS (300)
#define RPM (120)
#define MICROSTEPS (1)
#define STEPS (5)

#define YAW_BTN_PIN (32)     // Right pin in the RPi (GPIO18) (LEFT & RIGHT)
#define PITCH_BTN_PIN (33)   // Front pin in the RPi (GPIO17) (UP & DOWN)
#define START_GAME_PIN (31)  // coin switch pin in the RPi (GPIO25)
#define TOP_ROW_SENSOR_PIN (30)
#define MID_ROW_SENSOR_PIN (29)
#define BTM_ROW_SENSOR_PIN (28)

//clowns - count from left to right and top to bottom
#define CLOWN_1 (49)
#define CLOWN_2 (48)
#define CLOWN_3 (53)
#define CLOWN_4 (52)
#define CLOWN_5 (51)
#define CLOWN_6 (50)
#define CLOWN_7 (43)
#define CLOWN_8 (42)
#define CLOWN_9 (41)
#define CLOWN_10 (40)
#define CLOWN_11 (39)
#define CLOWN_12 (38)
#define CLOWN_13 (37)
#define CLOWN_14 (36)

#define RESET_GAME_MS (3000)
#define PITCH_UPDATE_MS (20)
#define PITCH_MIN (0)
#define PITCH_MAX (180)
#define YAW_UPDATE_MS (80)
#define PITCH_RESTART_POSITION (180)
#define YAW_MIN (0)
#define YAW_MAX (180)
#define YAW_RESTART_POSITION (180)
#define DELAY_MS (2000)

BasicStepperDriver stepper(MOTOR_STEPS, STEPPER_DIR_PIN, STEPPER_STEPS_PIN);

Button start_btn(START_GAME_PIN);  //gets coin from the RPi
Button yaw_btn(YAW_BTN_PIN);       // move "right"
Button pitch_btn(PITCH_BTN_PIN);   // move "left"
Button top_sensor(TOP_ROW_SENSOR_PIN);
Button mid_sensor(MID_ROW_SENSOR_PIN);
Button btm_sensor(BTM_ROW_SENSOR_PIN);
Timer reset_timer;
Timer yaw_update_timer;
Timer pitch_update_timer;
Timer delay_timer;  // timer to delay the blower's operation
Servo pitch;

uint8_t steps = STEPS;
uint16_t counter = 0;
volatile uint8_t yaw_position;
volatile uint8_t pitch_position;

// uint16_t clowns_mask = 0x0000;
uint8_t clowns_pins[] = {CLOWN_1, CLOWN_2, CLOWN_3, CLOWN_4, CLOWN_5, CLOWN_6, CLOWN_7, CLOWN_8, CLOWN_9, CLOWN_10, CLOWN_11, CLOWN_12, CLOWN_13, CLOWN_14};

//TODO: make it work without delay
void reset_nerf_position() {  //go left until the limit switch is pressed
    while (digitalRead(BTM_LIMIT_SWITCH_PIN)) {
        digitalWrite(STEPPER_DIR_PIN, HIGH);  // (HIGH = CCW / LOW = CW)
        digitalWrite(STEPPER_STEPS_PIN, HIGH);
        delay(5);  // Delay to slow down speed of Stepper
        digitalWrite(STEPPER_STEPS_PIN, LOW);
        delay(5);
    }
}

void limit_switches(bool state) {  //controls home sensors
    digitalWrite(LIMIT_SWITCH_1_PIN, state);
    digitalWrite(LIMIT_SWITCH_2_PIN, state);
}

uint16_t get_clowns_state() {
    uint16_t clowns_mask = 0;
    for (uint16_t i = 0; i < 14; i++) {
        clowns_mask |= (uint16_t)(digitalRead(clowns_pins[i]) << i);
    }

    return clowns_mask;
}

/*
uint16_t get_clowns_state() {
    uint16_t clowns_mask = 0;
    for (int i = 13; i >= 0; --i) {
        clowns_mask <<= 1;
        clowns_mask |= (digitalRead(clowns_pins[i]) == HIGH);
    }
    return clowns_mask;
}
*/

void winning_check(uint16_t mask) {
    if ((mask & 0b11111000000000) == 0b11111000000000) {  // btm row winning sequence
        digitalWrite(WINNING_SENSOR_PIN, HIGH);
        digitalWrite(BTM_ROW_MOTOR_PIN, LOW);  // =turn on
    } else {
        digitalWrite(WINNING_SENSOR_PIN, LOW);
        if (!digitalRead(BTM_ROW_SENSOR_PIN)) digitalWrite(BTM_ROW_MOTOR_PIN, HIGH);
    }
    if ((mask & 0b00000111110000) == 0b00000111110000) {  // mid row winning sequence
        digitalWrite(WINNING_SENSOR_PIN, HIGH);
        digitalWrite(MID_ROW_MOTOR_PIN, LOW);  // =turn on
    } else {
        digitalWrite(WINNING_SENSOR_PIN, LOW);
        if (!digitalRead(MID_ROW_SENSOR_PIN)) digitalWrite(MID_ROW_MOTOR_PIN, HIGH);
    }
    if ((mask & 0b00000000001111) == 0b00000000001111) {  // top row winning sequence
        digitalWrite(WINNING_SENSOR_PIN, HIGH);
        digitalWrite(TOP_ROW_MOTOR_PIN, LOW);  // =turn on
    } else {
        digitalWrite(WINNING_SENSOR_PIN, LOW);
        if (!digitalRead(TOP_ROW_SENSOR_PIN)) digitalWrite(TOP_ROW_MOTOR_PIN, HIGH);
    }
}

void game_start() {  // resets all parameters
    digitalWrite(WINNING_SENSOR_PIN, LOW);
    limit_switches(0);

    pitch_position = PITCH_RESTART_POSITION;
    pitch.write(pitch_position);  // restart pitch position

    yaw_position = YAW_MAX;
    reset_nerf_position();  // restart yaw position
}

void reset_cb() {
    limit_switches(0);
    digitalWrite(LAUNCHER_MOTOR_PIN, HIGH);
    digitalWrite(BLOWER_MOTOR_PIN, HIGH);
    digitalWrite(BELT_MOTOR_PIN, HIGH);

    pitch_position = PITCH_RESTART_POSITION;
    pitch.write(pitch_position);  // restart pitch position

    yaw_position = YAW_MAX;
    reset_nerf_position();  // restart yaw position
}

void yaw_update() {
    counter++;
    stepper.rotate(-steps);
    if (counter <= YAW_MIN || counter - 1 >= YAW_MAX) steps = -steps;
}

void pitch_update() {
    if (--pitch_position <= PITCH_MIN) pitch_position = PITCH_MIN + 1;
    pitch.write(pitch_position);
}

void delay_cb() {
    digitalWrite(BLOWER_MOTOR_PIN, LOW);
    reset_timer.start();
}

void setup() {
    Serial.begin(115200);
    start_btn.begin();
    yaw_btn.begin();
    pitch_btn.begin();
    stepper.begin(RPM, MICROSTEPS);

    pitch.attach(PITCH_SERVO_PIN);
    pitch.write(PITCH_RESTART_POSITION);

    reset_timer.setCallback(reset_cb);
    reset_timer.setTimeout(RESET_GAME_MS);

    yaw_update_timer.setCallback(yaw_update);
    yaw_update_timer.setInterval(YAW_UPDATE_MS);

    pitch_update_timer.setCallback(pitch_update);
    pitch_update_timer.setInterval(PITCH_UPDATE_MS);

    delay_timer.setCallback(delay_cb);  //timer to delay blower's operation
    delay_timer.setTimeout(DELAY_MS);

    //TODO: Add for loop to define inputs and outputs OR define them using port manipulation
    pinMode(TOP_ROW_SENSOR_PIN, INPUT);
    pinMode(MID_ROW_SENSOR_PIN, INPUT);
    pinMode(BTM_ROW_SENSOR_PIN, INPUT);
    pinMode(YAW_BTN_PIN, INPUT);
    pinMode(PITCH_BTN_PIN, INPUT);
    pinMode(START_GAME_PIN, INPUT);
    pinMode(BTM_LIMIT_SWITCH_PIN, INPUT_PULLUP);

    // DDRL |= B00000000;  //sets pins 49-42 as inputs
    for (uint8_t i = 0; i < 14; i++) {
        pinMode(clowns_pins[i], INPUT_PULLUP);
    }

    pinMode(TOP_ROW_MOTOR_PIN, OUTPUT);
    pinMode(MID_ROW_MOTOR_PIN, OUTPUT);
    pinMode(BTM_ROW_MOTOR_PIN, OUTPUT);
    pinMode(LAUNCHER_MOTOR_PIN, OUTPUT);
    pinMode(BLOWER_MOTOR_PIN, OUTPUT);
    pinMode(BELT_MOTOR_PIN, OUTPUT);
    pinMode(WINNING_SENSOR_PIN, OUTPUT);
    pinMode(LIMIT_SWITCH_1_PIN, OUTPUT);
    pinMode(LIMIT_SWITCH_2_PIN, OUTPUT);

    digitalWrite(WINNING_SENSOR_PIN, LOW);
    reset_cb();

    Serial.println(F(
        "________________________________\n"
        "\n"
        "  N E R F G U N  M a c h i n e  \n"
        "________________________________\n"
        "\n"
        "Made by KD Technology\n"
        "\n"));

    reset_nerf_position();
}

void loop() {
#ifdef DEBUG
    // Serial.println(mask, BIN);
    // delay(500);

    uint16_t state = get_clowns_state();
    for (uint16_t mask = (uint16_t)1 << 15; mask; mask >>= 1) {
        Serial.print((state & mask) ? '1' : '0');
    }
    Serial.println("\n");
    delay(500);
#else
    if (start_btn.pressed()) game_start();  //based on 1000us of the coin pin

    if (yaw_btn.pressed() || pitch_btn.pressed()) limit_switches(1);

    if (!digitalRead(YAW_BTN_PIN) && !yaw_update_timer.isRunning()) yaw_update_timer.start();
    if (yaw_btn.released() && yaw_update_timer.isRunning()) yaw_update_timer.stop();

    if (!digitalRead(PITCH_BTN_PIN) && !pitch_update_timer.isRunning()) {
        pitch_update_timer.start();
        digitalWrite(LAUNCHER_MOTOR_PIN, LOW);
    }

    if (pitch_btn.released() && pitch_update_timer.isRunning()) {
        pitch_update_timer.stop();
        delay_timer.start();
        digitalWrite(BELT_MOTOR_PIN, LOW);
    }

    winning_check(get_clowns_state());
    TimerManager::instance().update();
#endif
}
