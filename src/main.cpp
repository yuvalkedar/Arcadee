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
|   1   |   2   |   3   |
|-------|-------|-------|
|   4   |   5   |   6   |
|-------|-------|-------|
|   7   |   8   |   9   |
|-------|-------|-------|

*/

#include <Arduino.h>
#include <Button.h>  // https://github.com/madleech/Button


// #define DEBUG

#define BTN_1_PIN (2)
#define BTN_2_PIN (3)
#define BTN_3_PIN (4)
#define BTN_4_PIN (5)
#define BTN_5_PIN (6)
#define BTN_6_PIN (7)
#define BTN_7_PIN (8)
#define BTN_8_PIN (9)
#define BTN_9_PIN (10)
#define DATA_PIN (11)
#define CLK_PIN (12)
#define LATCH_PIN (13)

Button btn_1(BTN_1_PIN);
Button btn_2(BTN_2_PIN);
Button btn_3(BTN_3_PIN);
Button btn_4(BTN_4_PIN);
Button btn_5(BTN_5_PIN);
Button btn_6(BTN_6_PIN);
Button btn_7(BTN_7_PIN);
Button btn_8(BTN_8_PIN);
Button btn_9(BTN_9_PIN);


uint8_t char_array[9] = {96, 93, 117, 102, 55, 63, 97, 127, 119};   // without characters and zero (= 123)
uint16_t num_array[9] = {1, 2, 4, 8, 16, 32, 64, 128, 256};
uint8_t segment[7] = {0b01000000, 0b00000001, 0b00000010, 0b00000100, 0b00100000, 0b00010000, 0b00001000};
uint8_t btns_pins[9] = {BTN_1_PIN ,BTN_2_PIN ,BTN_3_PIN ,BTN_4_PIN ,BTN_5_PIN ,BTN_6_PIN ,BTN_7_PIN ,BTN_8_PIN ,BTN_9_PIN};
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

uint16_t get_btns_state() {
    uint16_t btns_mask = 0;
    for (uint16_t i = 0; i < 9; i++) {
        btns_mask |= (uint16_t)(digitalRead(btns_pins[i]) << i);
    }
    btns_mask = ~btns_mask - 65024;
    return btns_mask;   // Serial prints: 511 = no press, 510 = 1, 509 = 2, 507 = 3, 503 = 4, 495 = 5, 479 = 6, 447 = 7, 383 = 8, 255 = 9
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
    rand_digit_4 = random(0,9);
    rand_digit_3 = random(0,9);
    rand_digit_2 = random(0,9);
    rand_digit_1 = random(0,9);
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
            delay(1500);
            generate_code();
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
                state = DIGIT_1;
            }
            break;
    }
}

void setup() {
    Serial.begin(115200);
    btn_1.begin();
    btn_2.begin();
    btn_3.begin();
    btn_4.begin();
    btn_5.begin();
    btn_6.begin();
    btn_7.begin();
    btn_8.begin();
    btn_9.begin();
    pinMode(DATA_PIN, OUTPUT);  
    pinMode(LATCH_PIN, OUTPUT);
    pinMode (CLK_PIN, OUTPUT);

    for (uint8_t i = 0; i <= 8; i++) pinMode(btns_pins[i], INPUT_PULLUP);

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
    // Serial.println(digitalRead(SENS_1_PIN));
    // delay(500);
    uint16_t state = get_btns_state();
    for (uint16_t mask = (uint16_t)1 << 8; mask; mask >>= 1) {
        Serial.print((state & mask) ? '1' : '0');
    }
    Serial.println("\n");
    delay(50);
#else
    ser_input = Serial.read();
    if (ser_input == 'g') generate_code();
    update_code(get_btns_state());
#endif
}