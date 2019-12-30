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

// #define DEBUG    // TODO: add ifdef for debug to show targets positions in the serial monitor

//NOTICE: Relay's state in HIGH when pin is LOW
#define YAW_SERVO_PIN (5)
#define PITCH_SERVO_PIN (6)
#define LIMIT_SWITCH_2_PIN (16)     // limit switch r/l pin in the RPi (GPIO20)
#define LIMIT_SWITCH_1_PIN (17)     // limit switch f/b pin in the RPi (GPIO16)
#define CRAZY_CLOWN_START_PIN (18)  // the Crazy Clown machine needs to get a btn press after we enter coin and before the game starts
#define LAUNCHER_MOTOR_PIN (20)     // shooting motor
#define BLOWER_MOTOR_PIN (19)       // loads balls into the magazine
#define BELT_MOTOR_PIN (15)         // loads balls into the launcher
#define WINNING_SENSOR_PIN (22)     // winning switch pin in the RPi (GPIO12)
#define TOP_ROW_WIN_PIN (23)
#define MID_ROW_WIN_PIN (24)
#define BTM_ROW_WIN_PIN (25)
#define START_GAME_PIN (31)  // coin switch pin in the RPi (GPIO25)
#define YAW_BTN_PIN (32)     // Right pin in the RPi (GPIO18) (LEFT & RIGHT)
#define PITCH_BTN_PIN (33)   // Front pin in the RPi (GPIO17) (UP & DOWN)
#define TOP_ROW_FEEDBACK_PIN (30)
#define MID_ROW_FEEDBACK_PIN (29)
#define BTM_ROW_FEEDBACK_PIN (28)

//clowns - count from left to right and top to bottom
#define CLOWN_1 (47)
#define CLOWN_2 (46)
#define CLOWN_3 (45)
#define CLOWN_4 (44)
#define CLOWN_5 (43)
#define CLOWN_6 (42)
#define CLOWN_7 (41)
#define CLOWN_8 (40)
#define CLOWN_9 (39)
#define CLOWN_10 (38)
#define CLOWN_11 (37)
#define CLOWN_12 (36)
#define CLOWN_13 (35)
#define CLOWN_14 (34)

#define YAW_UPDATE_MS (40)
#define PITCH_UPDATE_MS (20)
#define RESET_GAME_MS (3000)  // TODO: change this interval
#define PITCH_MIN (0)
#define PITCH_MAX (180)
#define PITCH_RESTART_POSITION (180)
#define YAW_MIN (0)
#define YAW_MAX (180)
#define YAW_RESTART_POSITION (180)
#define DELAY_MS (2000)

Button start_btn(START_GAME_PIN);  //gets coin from the RPi
Button yaw_btn(YAW_BTN_PIN);       // move "right"
Button pitch_btn(PITCH_BTN_PIN);   // move "left"

Timer reset_timer;
Timer yaw_update_timer;
Timer pitch_update_timer;
Timer delay_timer;  // timer to delay the blower's operation

Servo yaw;
Servo pitch;

volatile uint8_t yaw_position;
volatile uint8_t pitch_position;

uint16_t clowns_mask = 0b00000000000000;

uint16_t get_clowns_state() {
    for (uint8_t i = 34; i < 48; i++) {
        //TODO: put the result in a bitmask and send this mask to is_winning_sequence()
        bitWrite(clowns_mask, i, digitalRead(i));
    }
    return clowns_mask;
}

bool is_winning_sequence(uint16_t mask) {
    /* did the given mask win the game? */
    if ((mask & 0b11110000000000) == 0b11110000000000) return true;
    if ((mask & 0b00001111100000) == 0b00001111100000) return true;
    if ((mask & 0b00000000011111) == 0b00000000011111) return true;
    return false;
}

void limit_switches(bool state) {  //controls home sensors
    digitalWrite(LIMIT_SWITCH_1_PIN, state);
    digitalWrite(LIMIT_SWITCH_2_PIN, state);
}

void winning_check() {
    // if (is_winning_sequence && digitalRead(TOP_ROW_FEEDBACK_PIN)) {  //NOTICE: need to make the feedback happen!
    if (is_winning_sequence(get_clowns_state())) {
        digitalWrite(TOP_ROW_WIN_PIN, LOW);
        digitalWrite(WINNING_SENSOR_PIN, HIGH);
    } else {
        digitalWrite(TOP_ROW_WIN_PIN, HIGH);
        digitalWrite(WINNING_SENSOR_PIN, LOW);
    }
}

void toggle_pin(uint8_t pin) {
    digitalWrite(pin, HIGH);  // toggling this pin to start a game...
    digitalWrite(pin, LOW);
}

void game_start() {                     // resets all parameters
    toggle_pin(CRAZY_CLOWN_START_PIN);  // toggling this pin to start a game...
    digitalWrite(WINNING_SENSOR_PIN, LOW);
    limit_switches(0);

    yaw_position = YAW_RESTART_POSITION;
    yaw.write(yaw_position);  // restart yaw position

    pitch_position = PITCH_RESTART_POSITION;
    pitch.write(pitch_position);  // restart pitch position
}

void reset_cb() {
    limit_switches(0);
    digitalWrite(LAUNCHER_MOTOR_PIN, HIGH);
    digitalWrite(BLOWER_MOTOR_PIN, HIGH);
    digitalWrite(BELT_MOTOR_PIN, HIGH);

    yaw_position = YAW_RESTART_POSITION;
    yaw.write(yaw_position);  // restart yaw position

    pitch_position = PITCH_RESTART_POSITION;
    pitch.write(pitch_position);  // restart pitch position
}

void yaw_update() {
    if (--yaw_position <= YAW_MIN) yaw_position = YAW_MIN + 1;  //TODO: change the "+1" to "+ defined parameter"
    yaw.write(yaw_position);
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

    yaw.attach(YAW_SERVO_PIN);
    yaw.write(YAW_RESTART_POSITION);
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

    //TODO: Add for loop to define inputs and outputs
    pinMode(TOP_ROW_FEEDBACK_PIN, INPUT);
    pinMode(MID_ROW_FEEDBACK_PIN, INPUT);
    pinMode(BTM_ROW_FEEDBACK_PIN, INPUT);
    pinMode(YAW_BTN_PIN, INPUT);
    pinMode(PITCH_BTN_PIN, INPUT);
    pinMode(START_GAME_PIN, INPUT);
    pinMode(CLOWN_1, INPUT);
    pinMode(CLOWN_2, INPUT);
    pinMode(CLOWN_3, INPUT);
    pinMode(CLOWN_4, INPUT);

    pinMode(TOP_ROW_WIN_PIN, OUTPUT);
    pinMode(MID_ROW_WIN_PIN, OUTPUT);
    pinMode(BTM_ROW_WIN_PIN, OUTPUT);
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
}

void loop() {
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

    winning_check();
    TimerManager::instance().update();
}
