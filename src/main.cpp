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
LAUNCHER_MOTOR_PIN, BLOWER_MOTOR_PIN
*/

// OUTPUTS
#define PITCH_SERVO_PIN (11)
#define STEPPER_STEPS_PIN (9)
#define STEPPER_DIR_PIN (10)
#define BTM_LIMIT_SWITCH_PIN (8)
#define LIMIT_SWITCH_2_PIN (16)  // limit switch r/l pin in the RPi (GPIO20)
#define LIMIT_SWITCH_1_PIN (17)  // limit switch f/b pin in the RPi (GPIO16)
#define LAUNCHER_MOTOR_PIN (20)  // shooting motor
#define BLOWER_MOTOR_PIN (19)    // loads balls into the magazine
#define BELT_MOTOR_DIR_PIN (13)      // loads balls into the launcher
#define BELT_MOTOR_SPEED_PIN (12)
#define WINNING_SENSOR_PIN (22)  // winning switch pin in the RPi (GPIO12)
#define TOP_ROW_MOTOR_PIN (23)
#define MID_ROW_MOTOR_PIN (24)
#define BTM_ROW_MOTOR_PIN (25)
// INPUTS
#define MOTOR_STEPS (126)
#define RPM (24)
#define MICROSTEPS (1)
#define STEPS (10)

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

#define RESET_GAME_MS (1000)
#define BLOWER_ON_MS (10000)
#define RESET_SHOOTING_MS (RESET_GAME_MS + 1200)
#define BELT_ON_MS (300 + RESET_GAME_MS)
#define PITCH_UPDATE_MS (20)
#define PITCH_MIN (0)
#define PITCH_MAX (180)
#define YAW_UPDATE_MS (80)
#define PITCH_RESTART_POSITION (180)
#define YAW_MIN (0)
#define YAW_MAX (90)
#define DEBOUNCE_DELAY_MS  (2000)

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
Timer blower_timer;
Timer belt_reverse_timer;
Timer shooting_timer;  //allows the shooting motor work a bit more after the loader stops in order to make sure balls won't get stuck in.
Servo pitch;

uint8_t steps = STEPS;
volatile int yaw_position;
volatile uint8_t pitch_position;

unsigned long last_debounce_time = 0;  // the last time the output pin was toggled
bool winning_state = false;

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

void move_belt_fw() {
    digitalWrite(BELT_MOTOR_SPEED_PIN, HIGH);
    digitalWrite(BELT_MOTOR_DIR_PIN, LOW);
}

void stop_belt() {
    digitalWrite(BELT_MOTOR_SPEED_PIN, HIGH);
    digitalWrite(BELT_MOTOR_DIR_PIN, HIGH);
}

void move_belt_bw() {
    digitalWrite(BELT_MOTOR_SPEED_PIN, LOW);
    digitalWrite(BELT_MOTOR_DIR_PIN, HIGH);
}


void winning_check(uint16_t mask) {
    if (((mask & 0b11111000000000) == 0b11111000000000 || (mask & 0b00000111110000) == 0b00000111110000 || (mask & 0b00000000001111) == 0b00000000001111) && (winning_state == false)) {
        Serial.println("Win detected");
        winning_state = true;
        last_debounce_time = millis();
        Serial.println(last_debounce_time);
    }
    if ((millis() - last_debounce_time) >= DEBOUNCE_DELAY_MS && (mask & 0b11111000000000) == 0b11111000000000) {  // btm row winning sequence
        Serial.println("btm row win confirmed");
        digitalWrite(WINNING_SENSOR_PIN, HIGH);
        digitalWrite(BTM_ROW_MOTOR_PIN, LOW);  // =turn on
    } else {
        // Serial.println(".");
        // digitalWrite(WINNING_SENSOR_PIN, LOW);
        if (!digitalRead(BTM_ROW_SENSOR_PIN)) digitalWrite(BTM_ROW_MOTOR_PIN, HIGH);
    }
    if ((millis() - last_debounce_time) >= DEBOUNCE_DELAY_MS && (mask & 0b00000111110000) == 0b00000111110000) {  // mid row winning sequence
        digitalWrite(WINNING_SENSOR_PIN, HIGH);
        digitalWrite(MID_ROW_MOTOR_PIN, LOW);  // =turn on
    } else {
        // digitalWrite(WINNING_SENSOR_PIN, LOW);
        if (!digitalRead(MID_ROW_SENSOR_PIN)) digitalWrite(MID_ROW_MOTOR_PIN, HIGH);
    }
    if ((millis() - last_debounce_time) >= DEBOUNCE_DELAY_MS && (mask & 0b00000000001111) == 0b00000000001111) {  // top row winning sequence
        digitalWrite(WINNING_SENSOR_PIN, HIGH);
        digitalWrite(TOP_ROW_MOTOR_PIN, LOW);  // =turn on
    } else {
        // digitalWrite(WINNING_SENSOR_PIN, LOW);
        if (!digitalRead(TOP_ROW_SENSOR_PIN)) digitalWrite(TOP_ROW_MOTOR_PIN, HIGH);
    }
}


/*
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
*/

void game_start() {  // resets all parameters
    digitalWrite(WINNING_SENSOR_PIN, LOW);
    limit_switches(0);

    pitch_position = PITCH_RESTART_POSITION;
    pitch.write(pitch_position);  // restart pitch position

    yaw_position = YAW_MIN;
    reset_nerf_position();  // restart yaw position
    winning_state = false;
}

void reset_cb() {
    limit_switches(0);
    // digitalWrite(LAUNCHER_MOTOR_PIN, HIGH);
    move_belt_fw();

    pitch_position = PITCH_RESTART_POSITION;
    pitch.write(pitch_position);  // restart pitch position

    yaw_position = YAW_MIN;
    winning_state = false;
    // reset_nerf_position();  // restart yaw position

    // digitalWrite(WINNING_SENSOR_PIN, LOW);
}

void shooting_reset_cb() {
    digitalWrite(LAUNCHER_MOTOR_PIN, HIGH);
}

void blower_off() {
    digitalWrite(BLOWER_MOTOR_PIN, HIGH);
}

void yaw_update() {
    stepper.rotate(-steps);
    yaw_position++;
    if (yaw_position <= YAW_MIN || yaw_position - 1 >= YAW_MAX) {
        steps = 0;
    } else
        steps = STEPS;
    // Serial.print("count: ");
    // Serial.println(yaw_position);
}

void pitch_update() {
    if (--pitch_position <= PITCH_MIN) pitch_position = PITCH_MIN + 1;
    pitch.write(pitch_position);
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

    blower_timer.setCallback(blower_off);
    blower_timer.setTimeout(BLOWER_ON_MS);

    belt_reverse_timer.setCallback(stop_belt);
    belt_reverse_timer.setTimeout(BELT_ON_MS);

    shooting_timer.setCallback(shooting_reset_cb);
    shooting_timer.setTimeout(RESET_SHOOTING_MS);

    yaw_update_timer.setCallback(yaw_update);
    yaw_update_timer.setInterval(YAW_UPDATE_MS);

    pitch_update_timer.setCallback(pitch_update);
    pitch_update_timer.setInterval(PITCH_UPDATE_MS);

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
    pinMode(BELT_MOTOR_DIR_PIN, OUTPUT);
    pinMode(BELT_MOTOR_SPEED_PIN, OUTPUT);
    pinMode(WINNING_SENSOR_PIN, OUTPUT);
    pinMode(LIMIT_SWITCH_1_PIN, OUTPUT);
    pinMode(LIMIT_SWITCH_2_PIN, OUTPUT);
    
    digitalWrite(WINNING_SENSOR_PIN, LOW);
    digitalWrite(BLOWER_MOTOR_PIN, HIGH);
    digitalWrite(LAUNCHER_MOTOR_PIN, HIGH);
    limit_switches(0);
    stop_belt();
    
    Serial.println(F(
        "________________________________\n"
        "\n"
        "  N E R F G U N  M a c h i n e  \n"
        "________________________________\n"
        "\n"
        "Made by KD Technology\n"
        "\n"));

    // reset_nerf_position();
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
    if (start_btn.pressed()) {
        // Serial.println("game starts");
        game_start();  //based on 1000us of the coin pin
    }

    // if (yaw_btn.pressed() || pitch_btn.pressed()) limit_switches(1);

    if (!digitalRead(YAW_BTN_PIN) && !yaw_update_timer.isRunning()) {
        // Serial.println("#1 btn is pressed");
        digitalWrite(LIMIT_SWITCH_2_PIN, HIGH);
        yaw_update_timer.start();
    }
    if (yaw_btn.released() && yaw_update_timer.isRunning()) yaw_update_timer.stop();

    if (!digitalRead(PITCH_BTN_PIN) && !pitch_update_timer.isRunning()) {
        // Serial.println("#2 btn is pressed");
        digitalWrite(LIMIT_SWITCH_1_PIN, HIGH);
        pitch_update_timer.start();
        digitalWrite(LAUNCHER_MOTOR_PIN, LOW);
    }

    if (pitch_btn.released() && pitch_update_timer.isRunning()) {
        pitch_update_timer.stop();
        move_belt_bw();
        digitalWrite(BLOWER_MOTOR_PIN, LOW);
        belt_reverse_timer.start();
        reset_timer.start();
        shooting_timer.start();
        blower_timer.start();
    }

    winning_check(get_clowns_state());
    TimerManager::instance().update();
#endif
}
