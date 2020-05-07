/*
    Automatic Basketball Machine

	Copyright (C) 2020 Yuval Kedar - KD Tech
    https://www.instagram.com/kd_technology/
	board: Arduino Nano
	
	Arduino Nano communicates with RPi.
	RPi sends button press commands (via app) to the Arduino, who sends commands to the machine.
*/

#include <Adafruit_NeoPixel.h>
#include <Button.h>  // https://github.com/madleech/Button
#include <Servo.h>
#include <timer.h>  // https://github.com/brunocalou/Timer
#include <timerManager.h>

#define AIMING_BTN_PIN (2)  // Front pin in the RPi (GPIO17)
#define LAUNCHER_BTN_PIN (4)  // Right pin in the RPi (GPIO18)
#define AIMING_SERVO_PIN (5)
#define MAGAZINE_SERVO_PIN (6)
#define WINNING_SENSOR_PIN (7)    // winning switch pin in the RPi (GPIO12)
#define START_GAME_PIN (8)        // coin switch pin in the RPi (GPIO25)
#define BLDC_1_PIN (9)
#define BLDC_2_PIN (10)
#define LIMIT_SWITCH_2_PIN (12)   // limit switch r/l pin in the RPi (GPIO20)
#define LIMIT_SWITCH_1_PIN (13)   // limit switch f/b pin in the RPi (GPIO16)
#define LED_BAR_PIN (A0)          // D14
#define MAGAZINE_SENSOR_PIN (A1)  // D15
#define BASKET_SENSOR_PIN (A2)    // D16
#define MAGAZINE_BLOWER_PIN (A5)  // D19 = air blower

#define NUM_PIXELS (8)
#define GAME_TIME (20000)  // in milliseconds
#define BASKET_SENSOR_LIMIT (500)
#define MAGAZINE_SENSOR_LIMIT (350)
#define LAUNCHER_DELAY_MS (500)
#define RESET_DELAY_MS (1500)
#define LED_BAR_BRIGHTNESS (250)
#define CANON_MIN (0)
#define CANON_MAX (180)
#define CALIBRATION_MS (1000)
#define CANON_STRENGTH (49)
#define LED_BAR_UPDATE_MS (300)
#define AIMING_UPDATE_MS (40)
#define AIMING_SERVO_MIN (20)
#define AIMING_SERVO_MAX (145)
#define MAGAZINE_LOADING_POSITION (90)
#define MAGAZINE_RESTART_POSITION (180)

Button start_btn(START_GAME_PIN);
Button aiming_btn(AIMING_BTN_PIN);
Button launcher_btn(LAUNCHER_BTN_PIN);

Timer launcher_timer;  //a "delay" after releasing the second button and before shooting (for canon motor to reach its speed)
Timer reset_timer;     //resets canon after shooting a ball
Timer strength_timer;  //updates the canon's strength and led_bar
Timer aiming_timer;    //updates the canon's position

Adafruit_NeoPixel strength_bar(NUM_PIXELS, LED_BAR_PIN, NEO_GRB + NEO_KHZ800);
Servo ESC_1;
Servo ESC_2;
Servo magazine;
Servo aiming;

volatile uint8_t led_bar = 0;
volatile uint32_t led_bar_colour[NUM_PIXELS] = {0x00cc00, 0x00cc00, 0x66cc00, 0xcccc00, 0xff9900, 0xff6600, 0xff3300, 0xff0000};

volatile uint8_t position;
int increment = 1;

void limit_switches(bool state) {
    digitalWrite(LIMIT_SWITCH_1_PIN, state);
    digitalWrite(LIMIT_SWITCH_2_PIN, state);
}

void winning_check() {
    (analogRead(BASKET_SENSOR_PIN) > BASKET_SENSOR_LIMIT) ? digitalWrite(WINNING_SENSOR_PIN, HIGH) : digitalWrite(WINNING_SENSOR_PIN, LOW);
}

void game_start() {  // resets all parameters
    limit_switches(0);
    position = AIMING_SERVO_MAX;
    aiming.write(position);  // restart aiming servo position
    magazine.write(MAGAZINE_RESTART_POSITION);
    ESC_1.write(CANON_MIN);
    ESC_2.write(CANON_MIN);
    led_bar = 0;
    digitalWrite(WINNING_SENSOR_PIN, LOW);
}

void reset_cb() {
    limit_switches(0);
    position = AIMING_SERVO_MAX;
    aiming.write(position);  // restart aiming servo position
    magazine.write(MAGAZINE_RESTART_POSITION);
    ESC_1.write(CANON_MIN);
    ESC_2.write(CANON_MIN);
    led_bar = 0;
    strength_bar.clear();
    strength_bar.show();
}

void launcher_cb() {  // when launcher button released
    magazine.write(MAGAZINE_LOADING_POSITION);
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
    ESC_1.attach(BLDC_1_PIN, 1000, 2000);
    ESC_2.attach(BLDC_2_PIN, 1000, 2000);
    ESC_1.write(CANON_MAX);
    ESC_2.write(CANON_MAX);
    delay(CALIBRATION_MS);
    ESC_1.write(CANON_MIN);
    ESC_2.write(CANON_MIN);
    delay(CALIBRATION_MS);
}

void canon_update() {
    strength_bar.setPixelColor(led_bar, led_bar_colour[led_bar]);
    strength_bar.show();

    if (++led_bar >= NUM_PIXELS) {
        led_bar = NUM_PIXELS - 1;
    }

    ESC_1.write((led_bar <= 5) ? CANON_STRENGTH : CANON_STRENGTH + 1);
    ESC_2.write((led_bar <= 5) ? CANON_STRENGTH : CANON_STRENGTH + 1);
}

/*
void aiming_update() {  // WITHOUT SWEEP
    if (--position <= AIMING_SERVO_MIN) position = AIMING_SERVO_MIN + 1;
    aiming.write(position);
}
*/

void aiming_update() {  // WITH SWEEP
    position -= increment;
    aiming.write(position);
    if (position <= AIMING_SERVO_MIN || position - 1 >= AIMING_SERVO_MAX) increment = -increment;
    Serial.println(position);
}

void setup() {
    Serial.begin(115200);
    canon_begin();
    start_btn.begin();
    launcher_btn.begin();
    aiming_btn.begin();
    strength_bar.begin();

    //TODO: add attach and detach when a game starts and ends.
    aiming.attach(AIMING_SERVO_PIN);
    aiming.write(AIMING_SERVO_MAX);  // restart aiming servo position
    magazine.attach(MAGAZINE_SERVO_PIN);
    magazine.write(MAGAZINE_RESTART_POSITION);

    launcher_timer.setCallback(launcher_cb);
    launcher_timer.setTimeout(LAUNCHER_DELAY_MS);

    reset_timer.setCallback(reset_cb);
    reset_timer.setTimeout(RESET_DELAY_MS);

    strength_timer.setCallback(canon_update);
    strength_timer.setInterval(LED_BAR_UPDATE_MS);

    aiming_timer.setCallback(aiming_update);
    aiming_timer.setInterval(AIMING_UPDATE_MS);

    pinMode(LAUNCHER_BTN_PIN, INPUT_PULLUP);
    pinMode(AIMING_BTN_PIN, INPUT_PULLUP);
    pinMode(START_GAME_PIN, INPUT);
    pinMode(BASKET_SENSOR_PIN, INPUT);
    pinMode(MAGAZINE_SENSOR_PIN, INPUT);
    pinMode(MAGAZINE_BLOWER_PIN, OUTPUT);
    pinMode(WINNING_SENSOR_PIN, OUTPUT);
    digitalWrite(WINNING_SENSOR_PIN, LOW);

    pinMode(LIMIT_SWITCH_1_PIN, OUTPUT);
    pinMode(LIMIT_SWITCH_2_PIN, OUTPUT);
    limit_switches(0);

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

    if (!digitalRead(AIMING_BTN_PIN) && !aiming_timer.isRunning()) {
        Serial.println("#1 btn is pressed");
        digitalWrite(LIMIT_SWITCH_2_PIN, HIGH);
        aiming_timer.start();
    }

    if (aiming_btn.released() && aiming_timer.isRunning()) aiming_timer.stop();

    if (!digitalRead(LAUNCHER_BTN_PIN) && !strength_timer.isRunning()) {
        Serial.println("#2 btn is pressed");
        digitalWrite(LIMIT_SWITCH_1_PIN, HIGH);
        strength_timer.start();
    }

    if (launcher_btn.released() && strength_timer.isRunning()) {
        launcher_timer.start();
        strength_timer.stop();
    }

    (analogRead(MAGAZINE_SENSOR_PIN) <= MAGAZINE_SENSOR_LIMIT) ? digitalWrite(MAGAZINE_BLOWER_PIN, LOW) : digitalWrite(MAGAZINE_BLOWER_PIN, HIGH);
    winning_check();

    TimerManager::instance().update();
}
