/*
* Author: Yuval Kedar - KD Technology
* Instagram: https://www.instagram.com/kd_technology/
* Date: Oct 19
* Dev board: Arduino Uno
*/

#include <FastLED.h>
#include <ezButton.h>
#include "Arduino.h"
#include <timer.h>  // https://github.com/brunocalou/Timer
#include <timerManager.h>

#define WINNING_SENSOR_PIN  (12)
#define LED_DATA_PIN        (6)
#define BLUE_BTN_PIN        (A0)
#define RED_BTN_PIN         (A3)

#define SERIAL_BAUDRATE     (115200)
#define NUM_LEDS            (64)
#define LED_BRIGHTNESS      (200)
#define WINNING_FX_TIME     (1000)
#define DELETE_TIME_MS      (1000)
#define DEBOUNCE_MS         (50)

ezButton blue_btn(BLUE_BTN_PIN);
ezButton red_btn(RED_BTN_PIN);
CRGB leds[NUM_LEDS];
Timer delete_level;

uint8_t score = 0;
uint8_t last_score = 0;
uint8_t level[] = {0, 28, 48, 60, 63};
uint8_t start_point = 0;
uint32_t cur_time;

void level_up(uint8_t led_num)
{
    if (led_num == level[1]) start_point = 0;   //up from level 0 to 1
    if (led_num == level[2]) start_point = 28;  //up from level 1 to 2
    if (led_num == level[3]) start_point = 48;  //up from level 2 to 3
    if (led_num == level[4]) start_point = 60;  //...

    for (uint8_t current_pixel = start_point; current_pixel < led_num; current_pixel++) leds[current_pixel] = CRGB::Blue;
    FastLED.show();
}

// Clear previous level frame & do the opposite direction effect with red color
void level_down(uint8_t led_num)
{
    if (led_num == level[0]) start_point = 28;  //down from level 1 to 0
    if (led_num == level[1]) start_point = 48;  //down from level 2 to 1
    if (led_num == level[2]) start_point = 60;  //down from level 3 to 2
    if (led_num == level[3]) start_point = 63;  //...

    for (int8_t i = start_point - 1; i >= led_num; i--) leds[i] = CRGB::Red;
    FastLED.show();
    delete_level.start();
}

void delete_level_cb()
{
    uint8_t led_num;
    if (start_point == 28) led_num = level[0];
    if (start_point == 48) led_num = level[1];
    if (start_point == 60) led_num = level[2];
    if (start_point == 63) led_num = level[3];

    for (int8_t i = start_point - 1; i >= led_num; i--) leds[i] = CRGB::Black;
    FastLED.show();
}

void fade_all()
{
    for (uint8_t i = 0; i < NUM_LEDS; i++) leds[i].nscale8(250);
}

void winning()
{
    static uint8_t hue = 0;
    for (uint8_t x = 0; x < 5; x++) {
        for (int8_t i = 0; i < NUM_LEDS; i++) {
            leds[i] = CHSV(hue++, 255, 255);
            FastLED.show(); 
            fade_all();
        }
        for (int8_t i = (NUM_LEDS)-1; i >= 0; i--) {
            leds[i] = CHSV(hue++, 255, 255);
            FastLED.show();
            fade_all();
        }
    }
}

void reset_game()
{
    score = 0;
    last_score = 4;
    digitalWrite(WINNING_SENSOR_PIN, LOW);
    FastLED.clear();
    FastLED.show();
}

void winning_check()
{
    (score == 4) ? analogWrite(WINNING_SENSOR_PIN, 175) : digitalWrite(WINNING_SENSOR_PIN, LOW);
}

void update_score()
{
    if (blue_btn.isPressed()) {
        Serial.println("+PLUS+");
        if (score++ >= 4) score = 4;
    }

    if (red_btn.isPressed()) {
        Serial.println("-MINUS-");
        if (score-- <= 0) score = 0;
    }

    if (score == 0){
        if (last_score == 1) level_down(level[0]);
        last_score = 0;
        digitalWrite(WINNING_SENSOR_PIN, LOW);
    }
    else if (score == 1) {
        if (last_score == 0) level_up(level[1]);    // if last_score was 0 make the blue effect because level is up
        if (last_score == 2) level_down(level[1]);  // if last_score was 2 make the red effect because level is down
        last_score = 1;
        digitalWrite(WINNING_SENSOR_PIN, LOW);
    }
    else if (score == 2) {
        if (last_score == 1) level_up(level[2]);
        if (last_score == 3) level_down(level[2]);
        last_score = 2;
        digitalWrite(WINNING_SENSOR_PIN, LOW);
    }
    else if (score == 3) {
        if (last_score == 2) level_up(level[3]);
        if (last_score == 4) level_down(level[3]);
        last_score = 3;
        digitalWrite(WINNING_SENSOR_PIN, LOW);
    }
    else if (score == 4) {
        winning_check();
        winning();  //this func makes issue when using ezButton.h. It calls "show" too many times.
        reset_game();
    }
}


void setup()
{
    Serial.begin(SERIAL_BAUDRATE);

    pinMode(WINNING_SENSOR_PIN, OUTPUT);
    digitalWrite(WINNING_SENSOR_PIN, LOW);

    delete_level.setCallback(delete_level_cb);
    delete_level.setTimeout(DELETE_TIME_MS);

    blue_btn.setDebounceTime(DEBOUNCE_MS);
    red_btn.setDebounceTime(DEBOUNCE_MS);

    FastLED.addLeds<NEOPIXEL, LED_DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
    FastLED.setBrightness(LED_BRIGHTNESS);
    FastLED.clear();
    FastLED.show();


    Serial.println(F(
        "_______________________________\n"
        "\n"
        "   G e a r   M a c h i n e     \n"
        "_______________________________\n"
        "\n"
        "   ~ Made by KD Technology ~   \n"
        "\n"));
}

void loop()
{
    blue_btn.loop();
    red_btn.loop();

    update_score();
    TimerManager::instance().update();
}
