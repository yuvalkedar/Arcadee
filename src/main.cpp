/*
	Author: Yuval Kedar - KD Technology
	Instagram: https://www.instagram.com/kd_technology/
	Date: Jun 20
	Dev board: Arduino Uno
	
	Safe Machine for Gigantic - Clawee.
    Hit the right code or get caught - then the code will regenerate itself.
	
	Arduino Uno communicates with RPi.

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


uint8_t char_array[9] = {96, 93, 117, 102, 55, 63, 97, 127, 119};   // without characters and zero (123)
uint8_t segment[7] = {0b01000000, 0b00000001, 0b00000010, 0b00000100, 0b00100000, 0b00010000, 0b00001000};
uint8_t btns_pins[9] = {BTN_1_PIN ,BTN_2_PIN ,BTN_3_PIN ,BTN_4_PIN ,BTN_5_PIN ,BTN_6_PIN ,BTN_7_PIN ,BTN_8_PIN ,BTN_9_PIN};
// uint8_t char_array[16] = {123, 96, 93, 117, 102, 55, 63, 97, 127, 119, 111, 62, 27, 124, 31, 15};   // with characters
long rand_digit_1;
long rand_digit_2;
long rand_digit_3;
long rand_digit_4;

uint16_t get_btns_state() {
    uint16_t btns_mask = 0;
    for (uint16_t i = 0; i < 9; i++) {
        btns_mask |= (uint16_t)(digitalRead(btns_pins[i]) << i);
    }
    
    return btns_mask;
}

void winning_check() {}

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
    rand_digit_1 = random(0,9);
    rand_digit_2 = random(0,9);
    rand_digit_3 = random(0,9);
    rand_digit_4 = random(0,9);
    digitalWrite(LATCH_PIN,LOW);
    shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,char_array[rand_digit_1]);
    shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,char_array[rand_digit_2]);
    shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,char_array[rand_digit_3]);
    shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,char_array[rand_digit_4]);
    digitalWrite(LATCH_PIN,HIGH);
    delay(3000);
}

void setup() {
    Serial.begin(115200);
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
}

void loop() {
    #ifndef DEBUG
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
    generate_code();
#endif
}