/*
	Author: Yuval Kedar - KD Technology
	Instagram: https://www.instagram.com/kd_technology/
	Date: Jul 19
	Dev board: Arduino Nano
	
	Canon ball controller for Gigantic - Clawee.
	
	Arduino Nano communicates with RPi.
	RPi sends button press commands (via app) to the Arduino, who sends commands to the machine.
    // TODO: RGB LEDs effects
*/

#include <Button.h>  // https://github.com/madleech/Button
#include <timer.h>   // https://github.com/brunocalou/Timer
#include <timerManager.h>
// #include "claweeCanon.h"
#include <Adafruit_NeoPixel.h>
#include "claweeServo.h"

#define LAUNCHER_BTN_PIN (2)  // Right pin in the RPi (GPIO18)
#define LAUNCHER_PIN (3)
#define AIMING_BTN_PIN (4)  // Front pin in the RPi (GPIO17)
#define AIMING_SERVO_PIN (5)
#define MAGAZINE_SERVO_PIN (6)
#define WINNING_PIN (7)     // winning switch pin in the RPi (GPIO12)
#define START_GAME_PIN (8)  // coin switch pin in the RPi (GPIO25)
// #define R_LED_PIN (9)
// #define G_LED_PIN (10)
// #define B_LED_PIN (11)
#define LIMIT_SWITCH_2_PIN (12)   // limit switch r/l pin in the RPi (GPIO20)
#define LIMIT_SWITCH_1_PIN (13)   // limit switch f/b pin in the RPi (GPIO16)
#define LED_BAR_PIN (A0)          // D14
#define MAGAZINE_SENSOR_PIN (A1)  // D15
#define BASKET_SENSOR_PIN (A2)    // D16
#define MAGAZINE_MOTOR_PIN (A5)   // D19 = air blower

#define NUM_PIXELS (8)
#define GAME_TIME (20000)  // in milliseconds
#define BASKET_SENSOR_LIMIT (500)
#define MAGAZINE_SENSOR_LIMIT (350)
#define LAUNCHER_DELAY_MS (1000)
#define LED_BAR_BRIGHTNESS (250)
#define SERVO_UPDATE_MS (40)
#define CANON_MIN (0)
#define CANON_MAX (180)
#define CALIBRATION_MS (1000)
#define CANON_STRENGTH (49)
#define UPDATE_MS (100)

Button start_btn(START_GAME_PIN);
Button aiming_btn(AIMING_BTN_PIN);
Button launcher_btn(LAUNCHER_BTN_PIN);

Timer launcher_timer;
Timer reset_timer;

ClaweeServo aiming_servo(SERVO_UPDATE_MS);
ClaweeServo magazine_servo(SERVO_UPDATE_MS);  // The time interval here doesn't really matter
Adafruit_NeoPixel strength_bar(NUM_PIXELS, LED_BAR_PIN, NEO_GRB + NEO_KHZ800);
Servo ESC;

uint32_t last_update;
uint8_t led_bar = 0;
uint32_t led_bar_colour[NUM_PIXELS] = {0x00cc00, 0x00cc00, 0x66cc00, 0xcccc00, 0xff9900, 0xff6600, 0xff3300, 0xff0000};
uint8_t increment = 1;

void limit_switches(bool state) {
    digitalWrite(LIMIT_SWITCH_1_PIN, state);
    digitalWrite(LIMIT_SWITCH_2_PIN, state);
}

void winning_check() {
    if (analogRead(BASKET_SENSOR_PIN) > BASKET_SENSOR_LIMIT) {
        digitalWrite(WINNING_PIN, HIGH);
        Serial.println(F("Congrats son, you might be the next Kobe Bryant!"));
    } else {
        digitalWrite(WINNING_PIN, LOW);
        // Serial.println(F("FUCKKK"));
    }
}

void game_start() {  // resets all parameters
    limit_switches(0);
    aiming_servo.Restart();
    magazine_servo.Magazine_restart();
    ESC.write(CANON_MIN);
    led_bar = 0;
    digitalWrite(WINNING_PIN, LOW);
}

void reset_cb() {
    aiming_servo.Restart();
    magazine_servo.Magazine_restart();
    ESC.write(CANON_MIN);
    led_bar = 0;
}

void launcher_cb() {  // when launcher button released
    magazine_servo.Load();
    reset_timer.start();
}

// Start a game only if there's a ball ready for shooting in the magazine
bool check_ball_loaded() {
    if (analogRead(MAGAZINE_SENSOR_PIN) <= MAGAZINE_SENSOR_LIMIT) {  // if there is no ball...
        limit_switches(1);
        /*
        Serial.println(F(
            "ALERT!\n"
            "Can't start a game. You ran out of bullets ;) \n"
            "\n"));
        */
        return 0;
    } else {
        limit_switches(0);
        // Serial.println(F("Ready for shooting!"));
        return 1;
    }
}

void canon_begin() {
    ESC.attach(LAUNCHER_PIN, 1000, 2000);
    ESC.write(CANON_MAX);
    delay(CALIBRATION_MS);
    ESC.write(CANON_MIN);
    delay(CALIBRATION_MS);
}

void canon_update() {
    if (millis() - last_update > UPDATE_MS * 3) {
        last_update = millis();
        strength_bar.setPixelColor(led_bar, led_bar_colour[led_bar]);
        strength_bar.show();
        led_bar += increment;
        if (led_bar <= 0 || led_bar >= 7) increment = -increment;
        if (led_bar <= 7) strength_bar.setPixelColor(led_bar + 1, 0x00);
        Serial.print(" led bar: ");
        Serial.println(led_bar);

        // NOTICE: ALWAYS SHOOTING AT THE SAME STRENGTH
        ESC.write(CANON_STRENGTH);
    }
}

void setup() {
    Serial.begin(115200);
    canon_begin();
    start_btn.begin();
    launcher_btn.begin();
    aiming_btn.begin();

    launcher_timer.setCallback(launcher_cb);
    launcher_timer.setTimeout(LAUNCHER_DELAY_MS);

    reset_timer.setCallback(reset_cb);
    reset_timer.setTimeout(LAUNCHER_DELAY_MS);

    aiming_servo.Attach(AIMING_SERVO_PIN);
    aiming_servo.Restart();
    magazine_servo.Attach(MAGAZINE_SERVO_PIN);
    magazine_servo.Magazine_restart();

    pinMode(LAUNCHER_BTN_PIN, INPUT);
    pinMode(AIMING_BTN_PIN, INPUT);
    pinMode(START_GAME_PIN, INPUT);
    pinMode(WINNING_PIN, INPUT);
    pinMode(BASKET_SENSOR_PIN, INPUT);
    pinMode(MAGAZINE_SENSOR_PIN, INPUT);

    pinMode(LIMIT_SWITCH_1_PIN, OUTPUT);
    pinMode(LIMIT_SWITCH_2_PIN, OUTPUT);
    limit_switches(0);

    pinMode(MAGAZINE_MOTOR_PIN, OUTPUT);

    Serial.println(F(
        "_______________________________\n"
        "\n"
        "   B a l l   L a u n c h e r   \n"
        "_______________________________\n"
        "\n"
        "Made by KD Technology\n"
        "\n"));
}

void loop() {
    if (check_ball_loaded()) {
        if (start_btn.pressed()) game_start();  //based on 1000us of the coin pin
    }
    if (aiming_btn.pressed() || launcher_btn.pressed()) limit_switches(1);

    if (!digitalRead(AIMING_BTN_PIN)) aiming_servo.Update();
    if (!digitalRead(LAUNCHER_BTN_PIN)) canon_update();  //TODO: add the led_bar with canon_update() and change strength +-1

    if (launcher_btn.released()) launcher_timer.start();

    (analogRead(MAGAZINE_SENSOR_PIN) <= MAGAZINE_SENSOR_LIMIT) ? digitalWrite(MAGAZINE_MOTOR_PIN, LOW) : digitalWrite(MAGAZINE_MOTOR_PIN, HIGH);
    winning_check();
    TimerManager::instance().update();
}