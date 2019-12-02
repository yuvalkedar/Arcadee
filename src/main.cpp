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

//TODO: change pin numbers
#define YAW_BTN_PIN (4)          // Right pin in the RPi (GPIO18) (LEFT & RIGHT)
#define PITCH_BTN_PIN (2)        // Front pin in the RPi (GPIO17) (UP & DOWN)
#define LAUNCHER_MOTOR_PIN (10)  // shooting motor
#define MAGAZINE_MOTOR_PIN (9)   // ball loading motor
#define YAW_SERVO_PIN (5)
#define PITCH_SERVO_PIN (6)
#define WINNING_SENSOR_PIN (7)   // winning switch pin in the RPi (GPIO12)
#define START_GAME_PIN (8)       // coin switch pin in the RPi (GPIO25)
#define LIMIT_SWITCH_2_PIN (12)  // limit switch r/l pin in the RPi (GPIO20)
#define LIMIT_SWITCH_1_PIN (13)  // limit switch f/b pin in the RPi (GPIO16)

#define GAME_TIME (20000)  // in milliseconds
#define YAW_UPDATE_MS (40)
#define PITCH_UPDATE_MS (40)
#define RESET_GAME_MS (1500)  // TODO: change this interval
#define PITCH_MIN (0)
#define PITCH_MAX (180)
#define PITCH_RESTART_POSITION (180)
#define YAW_MIN (0)
#define YAW_MAX (180)
#define YAW_RESTART_POSITION (180)

Button start_btn(START_GAME_PIN);
Button yaw_btn(YAW_BTN_PIN);
Button pitch_btn(PITCH_BTN_PIN);

Timer reset_timer;  //resets the canon after shooting a ball
Timer yaw_update;
Timer pitch_update;

Servo yaw;
Servo pitch;

volatile uint8_t yaw_position;
volatile uint8_t pitch_position;

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

// check if game_start() and reset_cb() are both needed

void game_start() {  // resets all parameters
    digitalWrite(WINNING_SENSOR_PIN, LOW);
    limit_switches(0);

    yaw_position = YAW_RESTART_POSITION;
    yaw.write(yaw_position);  // restart yaw position

    pitch_position = PITCH_RESTART_POSITION;
    yaw.write(pitch_position);  // restart pitch position
}

void reset_cb() {
    digitalWrite(LAUNCHER_MOTOR_PIN, LOW);
    digitalWrite(MAGAZINE_MOTOR_PIN, LOW);

    yaw_position = YAW_RESTART_POSITION;
    yaw.write(yaw_position);  // restart yaw position

    pitch_position = PITCH_RESTART_POSITION;
    yaw.write(pitch_position);  // restart pitch position
}

void yaw_update() {
    // strength_bar.setPixelColor(led_bar, led_bar_colour[led_bar]);
    // strength_bar.show();

    // if (++led_bar >= NUM_PIXELS) {
    //     led_bar = NUM_PIXELS - 1;
    // }

    // ESC.write((led_bar <= 5) ? CANON_STRENGTH : CANON_STRENGTH + 1);
}

void pitch_update() {
    // if (--position <= AIMING_SERVO_MIN) position = AIMING_SERVO_MIN + 1;
    // aiming.write(position);
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

    yaw_update.setCallback(yaw_update);
    yaw_update.setInterval(YAW_UPDATE_MS);

    pitch_update.setCallback(pitch_update);
    pitch_update.setInterval(PITCH_UPDATE_MS);

    pinMode(YAW_BTN_PIN, INPUT);
    pinMode(PITCH_BTN_PIN, INPUT);
    pinMode(START_GAME_PIN, INPUT);
    pinMode(LAUNCHER_MOTOR_PIN, OUTPUT);
    pinMode(MAGAZINE_MOTOR_PIN, OUTPUT);
    pinMode(WINNING_SENSOR_PIN, OUTPUT);
    digitalWrite(WINNING_SENSOR_PIN, LOW);

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

    if (!digitalRead(YAW_BTN_PIN) && !yaw_update.isRunning()) yaw_update.start();
    if (yaw_btn.released() && yaw_update.isRunning()) yaw_update.stop();

    if (!digitalRead(PITCH_BTN_PIN) && !pitch_update.isRunning()) pitch_update.start();

    if (pitch_btn.released() && pitch_update.isRunning()) pitch_update.stop();  //TODO: load balls (with timer?), shoot, and then call reset_timer.start();

    winning_check();
    TimerManager::instance().update();
}
