/*
  Author: Yuval Kedar - KD Technology
  Instagram: https://www.instagram.com/kd_technology/
  Date: Oct 19
  Dev board: Arduino Uno
  
  "Gear Machine" is a progress arcade machine for Gigantic.
  
  Arduino Uno communicates with RPi.
  A hamer (instead of a claw) hits buttons.
    There are two button types: Red and Blue.
    Red btn = -1
    Blue btn = +1

    On the machine's background there is an LED matrix which shows the user's progress.
    There are 4 levels (square frames) until one reaches the core and wins.

    The trick? The floor, which includes the buttons, is spinning.
*/

#include <Adafruit_NeoPixel.h>
#include <ezButton.h> // https://github.com/ArduinoGetStarted/button
#include <timer.h>   // https://github.com/brunocalou/Timer
#include <timerManager.h>
#include "Arduino.h"

#define WINNING_SENSOR_PIN (11)
#define LIMIT_SWITCH_PIN (12)
#define COIN_PIN (13)
#define LED_DATA_PIN (6)
#define MOTOR_PIN   (7)
#define BLUE_BTN_PIN (5)
#define RED_BTN_PIN (9)

#define SERIAL_BAUDRATE (115200)
#define NUM_LEDS (64)
#define LED_BRIGHTNESS (200)
#define WINNING_FX_TIME (2000)  //NOTICE: make sure the number isn't too big. User might start a new game before the effect ends.

Adafruit_NeoPixel strip(NUM_LEDS, LED_DATA_PIN, NEO_GRB + NEO_KHZ800);
ezButton blue_btn(BLUE_BTN_PIN);
ezButton red_btn(RED_BTN_PIN);
ezButton coin_btn(COIN_PIN);
ezButton home_btn(LIMIT_SWITCH_PIN);
Timer reset_timer;

int8_t score = 0;
uint8_t last_score = 0;
uint8_t level[] = {0, 28, 48, 60, 64};  //levels 0 to 4
uint8_t start_point = 0;
unsigned long last_update = 0;
int8_t i = 0;
int8_t j = 0;
uint8_t current_pixel = 0;

void delay_millis(uint32_t ms) {
    uint32_t start_ms = millis();
    while (millis() - start_ms < ms)
        ;
}

void level_up(uint8_t led_num) {
    if (led_num == level[1]) start_point = 0;   //up from level 0 to 1
    if (led_num == level[2]) start_point = 28;  //up from level 1 to 2
    if (led_num == level[3]) start_point = 48;  //up from level 2 to 3
    if (led_num == level[4]) start_point = 60;  //...

    for (current_pixel = start_point; current_pixel < led_num; current_pixel++) {
        strip.setPixelColor(current_pixel, strip.Color(0, 0, 255));
        strip.show();
        delay_millis(50);
    }
}

void level_down(uint8_t led_num) {  //clear prev level's frame and do the opposite direction effect with red color
    if (led_num == level[0]) start_point = 28;  //down from level 1 to 0
    if (led_num == level[1]) start_point = 48;  //down from level 2 to 1
    if (led_num == level[2]) start_point = 60;  //down from level 3 to 2
    if (led_num == level[3]) start_point = 64;  //...

    for (int8_t i = start_point - 1; i >= led_num; i--) {
        strip.setPixelColor(i, strip.Color(255, 0, 0));
        strip.show();
    }
    delay_millis(1000);
    for (int8_t i = start_point - 1; i >= led_num; i--) {
        strip.setPixelColor(i, strip.Color(0, 0, 0));
        strip.show();
    }
}

void winning() {  // Rainbow cycle
    for (long firstPixelHue = 0; firstPixelHue < 5 * 65536; firstPixelHue += 256) {
        for (uint8_t i = 0; i < strip.numPixels(); i++) {  // For each pixel in strip...
            int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
            strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
        }
        strip.show();
    }
}

void reset_game() {
    score = 0;
    last_score = 4;
    digitalWrite(WINNING_SENSOR_PIN, LOW);
    strip.clear();
    strip.show();
}

void winning_check() {
    (score == 4) ? analogWrite(WINNING_SENSOR_PIN, 175) : digitalWrite(WINNING_SENSOR_PIN, LOW);
}

void update_score() {
    if (blue_btn.isPressed()) {
        score++;
        Serial.println("PLUS");
        if (score >= 4) score = 4;
    }

    if (red_btn.isPressed()) {
        score--;
        Serial.println("MINUS");
        if (score <= 0) score = 0;
    }

    switch (score) {
        case 0:
            if (last_score == 1) level_down(level[0]);
            last_score = 0;
            digitalWrite(WINNING_SENSOR_PIN, LOW);
            break;
        case 1:
            if (last_score == 0) level_up(level[1]);    // if last_score was 0 make the blue effect because level is up
            if (last_score == 2) level_down(level[1]);  // if last_score was 2 make the red effect because level is down
            last_score = 1;
            digitalWrite(WINNING_SENSOR_PIN, LOW);
            // score = 1;
            break;
        case 2:
            if (last_score == 1) level_up(level[2]);
            if (last_score == 3) level_down(level[2]);
            last_score = 2;
            digitalWrite(WINNING_SENSOR_PIN, LOW);
            break;
        case 3:
            if (last_score == 2) level_up(level[3]);
            if (last_score == 4) level_down(level[3]);
            last_score = 3;
            digitalWrite(WINNING_SENSOR_PIN, LOW);
            break;
        case 4:
            winning_check();
            reset_timer.start();
            winning();
            break;
    }
}

void setup() {
    Serial.begin(SERIAL_BAUDRATE);

    pinMode(LED_DATA_PIN, OUTPUT);
    pinMode(WINNING_SENSOR_PIN, OUTPUT);
    pinMode(MOTOR_PIN, OUTPUT);

    digitalWrite(WINNING_SENSOR_PIN, LOW);
    digitalWrite(MOTOR_PIN, HIGH);

    blue_btn.setDebounceTime(30);
    red_btn.setDebounceTime(30);
    coin_btn.setDebounceTime(1);
    home_btn.setDebounceTime(250);

    reset_timer.setCallback(reset_game);
    reset_timer.setTimeout(WINNING_FX_TIME);

    strip.begin();
    strip.show();  // Turn OFF all pixels
    strip.setBrightness(LED_BRIGHTNESS);

    Serial.println(F(
        "_______________________________\n"
        "\n"
        "   G e a r   M a c h i n e     \n"
        "_______________________________\n"
        "\n"
        "   ~ Made by KD Technology ~   \n"
        "\n"));
}

void loop() {
    blue_btn.loop();
    red_btn.loop();
    coin_btn.loop();
    home_btn.loop();

    if(coin_btn.isPressed()) digitalWrite(MOTOR_PIN, LOW); // motor ON
    if (home_btn.isReleased()) digitalWrite(MOTOR_PIN, HIGH); // motor OFF

    update_score(); 
    TimerManager::instance().update();
}
