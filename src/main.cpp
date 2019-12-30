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
#define YAW_BTN_PIN (4)            // Right pin in the RPi (GPIO18) (LEFT & RIGHT)
#define PITCH_BTN_PIN (2)          // Front pin in the RPi (GPIO17) (UP & DOWN)
#define CRAZY_CLOWN_START_PIN (3)  // the Crazy Clown machine needs to get a btn press after we enter coin and before the game starts
#define LAUNCHER_MOTOR_PIN (11)    // shooting motor
#define BLOWER_MOTOR_PIN (10)      // loads balls into the magazine
#define BELT_MOTOR_PIN (9)         // loads balls into the launcher
#define YAW_SERVO_PIN (5)
#define PITCH_SERVO_PIN (6)
#define WINNING_SENSOR_PIN (8)   // winning switch pin in the RPi (GPIO12)
#define START_GAME_PIN (7)       // coin switch pin in the RPi (GPIO25)
#define LIMIT_SWITCH_2_PIN (12)  // limit switch r/l pin in the RPi (GPIO20)
#define LIMIT_SWITCH_1_PIN (13)  // limit switch f/b pin in the RPi (GPIO16)
#define TOP_ROW_WIN_PIN (A0)
#define MID_ROW_WIN_PIN (A1)
#define BTM_ROW_WIN_PIN (A2)

//clowns - count from left to right and top to bottom
#define CLOWN_1 (47)
#define CLOWN_2 (46)
#define CLOWN_3 (45)
#define CLOWN_4 (44)

#define YAW_UPDATE_MS (40)
#define PITCH_UPDATE_MS (20)
#define RESET_GAME_MS (1500)  // TODO: change this interval
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

bool clowns[3][5] = {{CLOWN_1, CLOWN_2, CLOWN_3, CLOWN_4, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}};

void limit_switches(bool state) {  //controls home sensors
    digitalWrite(LIMIT_SWITCH_1_PIN, state);
    digitalWrite(LIMIT_SWITCH_2_PIN, state);
}

void winning_check() {
    // (top_row.pressed() || mid_row.pressed() || btm_row.pressed()) ? digitalWrite(WINNING_SENSOR_PIN, HIGH) : digitalWrite(WINNING_SENSOR_PIN, LOW);
    if (CLOWN_1 && CLOWN_2 && CLOWN_3 && CLOWN_4) {
        digitalWrite(TOP_ROW_WIN_PIN, HIGH);  //NOTICE: It will need a toggle and also to toggle CRAZY_CLOWN_START_PIN
        digitalWrite(WINNING_SENSOR_PIN, HIGH);
    }
}

void toggle_pin(uint8_t pin_num) {
    digitalWrite(pin_num, HIGH);  // toggling this pin to start a game...
    digitalWrite(pin_num, LOW);
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

    digitalWrite(WINNING_SENSOR_PIN, LOW);
    digitalWrite(LAUNCHER_MOTOR_PIN, HIGH);
    digitalWrite(BLOWER_MOTOR_PIN, HIGH);
    digitalWrite(BELT_MOTOR_PIN, HIGH);

    pinMode(LIMIT_SWITCH_1_PIN, OUTPUT);
    pinMode(LIMIT_SWITCH_2_PIN, OUTPUT);
    limit_switches(0);

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
