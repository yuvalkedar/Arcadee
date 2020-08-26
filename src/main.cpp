/*
  Safe Machine for Gigantic - Clawee.
    Hit the correct code or get caught - then the code will regenerate itself.
    ========================================================================
  Copyright (C) 2020 Yuval Kedar - KD Tech
  Instagram: https://www.instagram.com/kd_technology/
  Date: Jun 20
  Dev board: Arduino Uno
        A
    ----------
   |          |
 B |          | G
   |    C     |
    ----------
   |          |
 D |          | F
   |          |
    ----------
        E       * DP
DP, G, F, E, D, C, B, A
01111011 - 0
01100000 - 1
01011101 - 2
01110101 - 3
01100110 - 4
00110111 - 5
00111111 - 6
01100001 - 7
01111111 - 8
01110111 - 9
01101111 - A
00011011 - C
00111110 - B
01111100 - D
00011111 - E
00001111 - F
*/

#include <Arduino.h>
#include <Button.h>  // https://github.com/madleech/Button
#include <Servo.h>
#include <timer.h>  // https://github.com/brunocalou/Timer
#include <timerManager.h>
#include <ShiftRegister74HC595.h>

// #define DEBUG

#define BTN_1_PIN (A1)
#define BTN_2_PIN (A4)
#define BTN_3_PIN (A7)
#define BTN_4_PIN (A0)
#define BTN_5_PIN (A2)
#define BTN_6_PIN (A5)
#define BTN_7_PIN (A3)
#define BTN_8_PIN (A6)
#define SECOND_BTN_PIN (2)  // Front pin in the RPi (GPIO17)
#define BALL_SERVO_PIN (3)
#define CLAW_BTN_PIN (4)
#define WIN_SERVO_PIN (5)
#define BLOWER_PIN (6)
#define WINNING_SENSOR_PIN (7)  // winning switch pin in the RPi (GPIO12)
#define START_GAME_PIN (8)        // coin switch pin in the RPi (GPIO25)
#define LATCH_PIN (9)
#define CLK_PIN (10)
#define DATA_PIN (11)

#define SENS_1_THRESHOLD (620)
#define SENS_2_THRESHOLD (600)
#define SENS_3_THRESHOLD (550)
#define SENS_4_THRESHOLD (600)
#define SENS_5_THRESHOLD (650)
#define SENS_6_THRESHOLD (600)
#define SENS_7_THRESHOLD (550)
#define SENS_8_THRESHOLD (650)
#define WIN_SERVO_MAX (10)
#define WIN_SERVO_MIN (130)
#define BALL_SERVO_MAX (180)    //OPEN POAITION
#define BALL_SERVO_MIN (100)     //CLOSE POSITION
#define WIN_RESET_DELAY_MS (3000)
#define GAME_RESET_DELAY_MS (1000)
#define BLOWER_DELAY_MS (9000)
#define WIN_SERVO_DELAY_MS (150)
#define DIGITS_COUNT (4)

Servo win_servo;
Servo ball_servo;
Timer win_reset_timer;
Timer game_reset_timer;
Timer blower_timer;
Button second_btn(SECOND_BTN_PIN);
Button start_btn(START_GAME_PIN);
ShiftRegister74HC595<DIGITS_COUNT> sr(DATA_PIN, CLK_PIN, LATCH_PIN);

uint8_t char_array[9] = {96, 93, 117, 102, 55, 63, 97, 127};   // without characters, zero (= 123), and nine (?=119)
uint16_t num_array[9] = {1, 2, 4, 8, 16, 32, 64, 128, 256};
uint8_t segment[7] = {0b01000000, 0b00000001, 0b00000010, 0b00000100, 0b00100000, 0b00010000, 0b00001000};
uint8_t btns_pins[8] = {BTN_1_PIN ,BTN_2_PIN ,BTN_3_PIN ,BTN_4_PIN ,BTN_5_PIN ,BTN_6_PIN ,BTN_7_PIN ,BTN_8_PIN};
uint16_t sensor_threshold[8] = {SENS_1_THRESHOLD, SENS_2_THRESHOLD, SENS_3_THRESHOLD, SENS_4_THRESHOLD, SENS_5_THRESHOLD, SENS_6_THRESHOLD, SENS_7_THRESHOLD, SENS_8_THRESHOLD};
uint8_t nice_array[4] = {0b01101011, 0b01100000, 0b00011011, 0b00011111};   // "nice""
long rand_digit_4;
long rand_digit_3;
long rand_digit_2;
long rand_digit_1;
uint8_t digits_buff[DIGITS_COUNT];

enum State {
  DIGIT_1 = 1,
  DIGIT_2 = 2,
  DIGIT_3 = 3,
  DIGIT_4 = 4
};
uint8_t state = DIGIT_1;

uint16_t get_sensors_state() {
    uint16_t btns_mask = 0;
    for (uint16_t i = 0; i < sizeof(btns_pins); i++) {
        if (analogRead(btns_pins[i]) <= sensor_threshold[i]){
            btns_mask |= 1 << i;
        }
    }
    // Serial.println(btns_mask);   // Serial prints: 0 = no sens, 1 = 1, 2 = 2, 4 = 3, 8 = 4, 16 = 5, 32 = 6, 64 = 7, 128 = 8
    return btns_mask;
}

void delay_millis(uint32_t ms) {
    uint32_t start_ms = millis();
    while (millis() - start_ms < ms);
}

void generate_code() {
    for (uint8_t d = 60; d > 0; d -= 5) {
        for (uint8_t i = 0; i < 8; i++) {
            digits_buff[0] = segment[i];
            digits_buff[1] = segment[i];
            digits_buff[2] = segment[i];
            digits_buff[3] = segment[i];
            sr.setAll(digits_buff);
            delay_millis(d);  
        }
    }
    rand_digit_4 = random(0,8);
    rand_digit_3 = random(0,8);
    rand_digit_2 = random(0,8);
    rand_digit_1 = random(0,8);
    digits_buff[0] = char_array[rand_digit_4];
    digits_buff[1] = char_array[rand_digit_3];
    digits_buff[2] = char_array[rand_digit_2];
    digits_buff[3] = char_array[rand_digit_1];
    sr.setAll(digits_buff);
}

void write_nice(){
    digits_buff[0] = nice_array[0];
    digits_buff[1] = nice_array[1];
    digits_buff[2] = nice_array[2];
    digits_buff[3] = nice_array[3];
    sr.setAll(digits_buff);
}

void update_win_servo(bool dir) {   //dir 1 = open, dir 0 = close
    if (dir) {
        for (uint8_t i = WIN_SERVO_MAX; i <= WIN_SERVO_MIN; i += 5) {
            win_servo.write(i);
            delay_millis(WIN_SERVO_DELAY_MS);
        }
    } else {
        for (uint8_t i = WIN_SERVO_MIN; i >= WIN_SERVO_MAX; i -= 5) {
            win_servo.write(i);
            delay_millis(WIN_SERVO_DELAY_MS);
        }
    }
}

void update_code(uint16_t mask) {
    switch(state) {
        case DIGIT_1:
            digitalWrite(WINNING_SENSOR_PIN, LOW);
            if (mask == num_array[rand_digit_4]) {
                digits_buff[0] = 0;
                digits_buff[1] = char_array[rand_digit_3];
                digits_buff[2] = char_array[rand_digit_2];
                digits_buff[3] = char_array[rand_digit_1];
                sr.setAll(digits_buff);
                delay(2000);    // Delay overcomes faulty press when two or more following numbers are identical
                state = DIGIT_2;
            }
            break;
        case DIGIT_2:
            digitalWrite(WINNING_SENSOR_PIN, LOW);
            if (mask == num_array[rand_digit_3]) {
                digits_buff[0] = 0;
                digits_buff[1] = 0;
                digits_buff[2] = char_array[rand_digit_2];
                digits_buff[3] = char_array[rand_digit_1];
                sr.setAll(digits_buff);
                delay(2000);
                state = DIGIT_3;
            }
            break;
        case DIGIT_3:
            digitalWrite(WINNING_SENSOR_PIN, LOW);
            if (mask == num_array[rand_digit_2]) {
                digits_buff[0] = 0;
                digits_buff[1] = 0;
                digits_buff[2] = 0;
                digits_buff[3] = char_array[rand_digit_1];
                sr.setAll(digits_buff);
                delay(2000);
                state = DIGIT_4;
            }
            break;
        case DIGIT_4:
            if (mask == num_array[rand_digit_1]) {
                digits_buff[0] = 0;
                digits_buff[1] = 0;
                digits_buff[2] = 0;
                digits_buff[3] = 0;
                sr.setAll(digits_buff);
                win_reset_timer.start();
                ball_servo.write(BALL_SERVO_MIN);
                update_win_servo(1);
                digitalWrite(WINNING_SENSOR_PIN, HIGH);
                write_nice();
                // Serial.println("YOU WON");
                delay(2000);
                state = DIGIT_1;
            } else
                digitalWrite(WINNING_SENSOR_PIN, LOW);
            break;
    }
}

void win_reset_cb(){
    update_win_servo(0);    //closes the door
    generate_code();
    digitalWrite(WINNING_SENSOR_PIN, LOW);
}

void game_reset_cb() {
    digitalWrite(CLAW_BTN_PIN,HIGH);
    ball_servo.write(BALL_SERVO_MIN);
    blower_timer.start();
}

void blower_reset_cb() {
    digitalWrite(BLOWER_PIN, HIGH);
}

void setup() {
    Serial.begin(115200);

    win_servo.attach(WIN_SERVO_PIN);
    win_servo.write(WIN_SERVO_MAX);  // restart win servo position

    ball_servo.attach(BALL_SERVO_PIN);
    ball_servo.write(BALL_SERVO_MIN);  // restart ball servo position

    win_reset_timer.setCallback(win_reset_cb);
    win_reset_timer.setTimeout(WIN_RESET_DELAY_MS);

    game_reset_timer.setCallback(game_reset_cb);
    game_reset_timer.setTimeout(GAME_RESET_DELAY_MS);

    blower_timer.setCallback(blower_reset_cb);
    blower_timer.setTimeout(BLOWER_DELAY_MS);

    pinMode(DATA_PIN, OUTPUT);  
    pinMode(LATCH_PIN, OUTPUT);
    pinMode(CLK_PIN, OUTPUT);
    pinMode(WINNING_SENSOR_PIN, OUTPUT);
    pinMode(CLAW_BTN_PIN, OUTPUT);
    pinMode(BLOWER_PIN, OUTPUT);

    pinMode(BTN_1_PIN, INPUT);
    pinMode(BTN_2_PIN, INPUT);
    pinMode(BTN_3_PIN, INPUT);
    pinMode(BTN_4_PIN, INPUT);
    pinMode(BTN_5_PIN, INPUT);
    pinMode(BTN_6_PIN, INPUT);
    pinMode(BTN_7_PIN, INPUT);
    pinMode(BTN_8_PIN, INPUT);
    pinMode(SECOND_BTN_PIN, INPUT_PULLUP);
    pinMode(START_GAME_PIN, INPUT);

    digitalWrite(WINNING_SENSOR_PIN, LOW);
    digitalWrite(BLOWER_PIN, HIGH);
    digitalWrite(DATA_PIN,LOW);
    digitalWrite(LATCH_PIN,LOW);
    digitalWrite(CLK_PIN,LOW);
    digitalWrite(CLAW_BTN_PIN,HIGH);

    randomSeed(analogRead(0));

    Serial.println(F(
    "________________________________\n"
    "\n"
    "     S A F E   M a c h i n e    \n"
    "________________________________\n"
    "\n"
    "     Made by KD Technology      \n"
    "\n"));

    generate_code();
}

void loop() {
#ifdef DEBUG
    Serial.print(analogRead(BTN_1_PIN));
    Serial.print("\t");
    Serial.print(analogRead(BTN_2_PIN));
    Serial.print("\t");
    Serial.print(analogRead(BTN_3_PIN));
    Serial.print("\t");
    Serial.print(analogRead(BTN_4_PIN));
    Serial.print("\t");
    Serial.print(analogRead(BTN_5_PIN));
    Serial.print("\t");
    Serial.print(analogRead(BTN_6_PIN));
    Serial.print("\t");
    Serial.print(analogRead(BTN_7_PIN));
    Serial.print("\t");
    Serial.println(analogRead(BTN_8_PIN));
    delay_millis(500);
#else
    update_code(get_sensors_state());

    if (start_btn.pressed()) ball_servo.write(BALL_SERVO_MIN);

    if (second_btn.released()) {
        ball_servo.write(BALL_SERVO_MAX);
        digitalWrite(CLAW_BTN_PIN,LOW);
        digitalWrite(BLOWER_PIN, LOW);
        game_reset_timer.start();
    }

    TimerManager::instance().update();
#endif
}
