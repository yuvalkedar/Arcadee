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

BTNS KEYBOARD:

|-------|-------|-------|
|       |   1   |       |
|-------|-------|-------|
|    2  |   3   |   4   |
|-------|-------|-------|
|     5 |       | 6     |
|-------|-------|-------|
| 7     |   8   |     9 |
|-------|-------|-------|

*/

#include <Arduino.h>
#include <Servo.h>
#include <timer.h>  // https://github.com/brunocalou/Timer
#include <timerManager.h>

// #define DEBUG

#define BTN_1_PIN (A0)
#define BTN_2_PIN (A1)
#define BTN_3_PIN (A2)
#define BTN_4_PIN (A3)
#define BTN_5_PIN (A4)
#define BTN_6_PIN (A5)
#define BTN_7_PIN (A6)
#define BTN_8_PIN (A7)
#define WIN_SERVO_PIN (5)

#define DATA_PIN (11)
#define CLK_PIN (10)
#define LATCH_PIN (9)

#define SENS_1_THRESHOLD (700)
#define SENS_2_THRESHOLD (700)
#define SENS_3_THRESHOLD (750)
#define SENS_4_THRESHOLD (700)
#define SENS_5_THRESHOLD (700)
#define SENS_6_THRESHOLD (700)
#define SENS_7_THRESHOLD (700)
#define SENS_8_THRESHOLD (700)
#define WIN_SERVO_MAX (10)
#define WIN_SERVO_MIN (130)
#define RESET_DELAY_MS (3000)
#define WIN_SERVO_DELAY_MS (150)

Servo win_servo;
Timer reset_timer;

uint8_t char_array[9] = {96, 93, 117, 102, 55, 63, 97, 127};   // without characters, zero (= 123), and nine (?=119)
uint16_t num_array[9] = {1, 2, 4, 8, 16, 32, 64, 128, 256};
uint8_t segment[7] = {0b01000000, 0b00000001, 0b00000010, 0b00000100, 0b00100000, 0b00010000, 0b00001000};
uint8_t btns_pins[8] = {BTN_1_PIN ,BTN_2_PIN ,BTN_3_PIN ,BTN_4_PIN ,BTN_5_PIN ,BTN_6_PIN ,BTN_7_PIN ,BTN_8_PIN};
uint16_t sensor_threshold[8] = {SENS_1_THRESHOLD, SENS_2_THRESHOLD, SENS_3_THRESHOLD, SENS_4_THRESHOLD, SENS_5_THRESHOLD, SENS_6_THRESHOLD, SENS_7_THRESHOLD, SENS_8_THRESHOLD};
// uint8_t char_array[16] = {123, 96, 93, 117, 102, 55, 63, 97, 127, 119, 111, 62, 27, 124, 31, 15};   // with characters
long rand_digit_4;
long rand_digit_3;
long rand_digit_2;
long rand_digit_1;
char ser_input;

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

void update_win_servo(bool dir) {   //dir 1 = open, dir 0 = close
    if (dir) {
        for (uint8_t i = WIN_SERVO_MAX; i <= WIN_SERVO_MIN; i += 5) {
            win_servo.write(i);
            delay(WIN_SERVO_DELAY_MS);
        }
    } else {
        for (uint8_t i = WIN_SERVO_MIN; i >= WIN_SERVO_MAX; i -= 5) {
            win_servo.write(i);
            delay(WIN_SERVO_DELAY_MS);
        }
    }
}

void generate_code() {
    for (uint8_t d = 60; d > 0; d -= 5) {
        for (uint8_t i = 0; i < 8; i++) {
            digitalWrite(LATCH_PIN,LOW);
            shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,segment[i]);
            shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,segment[i]);
            shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,segment[i]);
            shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,segment[i]);
            digitalWrite(LATCH_PIN,HIGH);
            delay(d);  
        }
    }
    rand_digit_4 = random(0,8);
    rand_digit_3 = random(0,8);
    rand_digit_2 = random(0,8);
    rand_digit_1 = random(0,8);
    digitalWrite(LATCH_PIN,LOW);
    shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,char_array[rand_digit_4]);
    shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,char_array[rand_digit_3]);
    shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,char_array[rand_digit_2]);
    shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,char_array[rand_digit_1]);
    digitalWrite(LATCH_PIN,HIGH);
}

void delete_digit(uint8_t digit) {
    switch (digit) {
        case 1:
            digitalWrite(LATCH_PIN,LOW);
            shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,char_array[rand_digit_4]);
            shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,char_array[rand_digit_3]);
            shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,char_array[rand_digit_2]);
            shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,0);
            digitalWrite(LATCH_PIN,HIGH);
            break;
        case 2:
            digitalWrite(LATCH_PIN,LOW);
            shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,char_array[rand_digit_4]);
            shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,char_array[rand_digit_3]);
            shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,0);
            shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,0);
            digitalWrite(LATCH_PIN,HIGH);
            break;
        case 3:
            digitalWrite(LATCH_PIN,LOW);
            shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,char_array[rand_digit_4]);
            shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,0);
            shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,0);
            shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,0);
            digitalWrite(LATCH_PIN,HIGH);
            break;
        case 4:
            digitalWrite(LATCH_PIN,LOW);
            shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,0);
            shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,0);
            shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,0);
            shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,0);
            digitalWrite(LATCH_PIN,HIGH);
            break;
    }
}

void update_code(uint16_t mask) {
    switch(state) {
        case DIGIT_1:
            if (mask == num_array[rand_digit_1]) {
                delete_digit(1);
                delay(4000);    // Delay overcomes faulty press when two or more following numbers are identical
                state = DIGIT_2;
            }
            break;
        case DIGIT_2:
            if (mask == num_array[rand_digit_2]) {
                delete_digit(2);
                delay(4000);
                state = DIGIT_3;
            }
            break;
        case DIGIT_3:
            if (mask == num_array[rand_digit_3]) {
                delete_digit(3);
                delay(4000);
                state = DIGIT_4;
            }
            break;
        case DIGIT_4:
            if (mask == num_array[rand_digit_4]) {
                delete_digit(4);
                update_win_servo(1);
                reset_timer.start();
                state = DIGIT_1;
            }
            break;
    }
}

void reset_cb(){
        update_win_servo(0);    //closes the door
        generate_code();
}

void setup() {
    Serial.begin(115200);

    win_servo.attach(WIN_SERVO_PIN);
    win_servo.write(WIN_SERVO_MAX);  // restart win servo position

    reset_timer.setCallback(reset_cb);
    reset_timer.setTimeout(RESET_DELAY_MS);

    pinMode(DATA_PIN, OUTPUT);  
    pinMode(LATCH_PIN, OUTPUT);
    pinMode(CLK_PIN, OUTPUT);

    pinMode(BTN_1_PIN, INPUT);
    pinMode(BTN_2_PIN, INPUT);
    pinMode(BTN_3_PIN, INPUT);
    pinMode(BTN_4_PIN, INPUT);
    pinMode(BTN_5_PIN, INPUT);
    pinMode(BTN_6_PIN, INPUT);
    pinMode(BTN_7_PIN, INPUT);
    pinMode(BTN_8_PIN, INPUT);

    // for (uint8_t i = 0; i <= 8; i++) pinMode(btns_pins[i], INPUT_PULLUP);

    digitalWrite(DATA_PIN,LOW);
    digitalWrite(LATCH_PIN,LOW);
    digitalWrite(CLK_PIN,LOW);

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
    // Serial.println(mask, BIN);
    // delay(500);

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
    delay(500);
    
    // get_sensors_state();
    // delay(500);
#else
    ser_input = Serial.read();
    if (ser_input == 'g') generate_code();
    update_code(get_sensors_state());

    TimerManager::instance().update();
#endif
}