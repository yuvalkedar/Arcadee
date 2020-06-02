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

*/

#include <Arduino.h>

#define CLK_PIN (10)
#define DATA_PIN (11)
#define LATCH_PIN (13)


uint8_t char_array[10] = {123, 96, 93, 117, 102, 55, 63, 97, 127, 119};   // without characters
uint8_t segment[7] = {0b01000000, 0b00000001, 0b00000010, 0b00000100, 0b00100000, 0b00010000, 0b00001000};
// uint8_t char_array[16] = {123, 96, 93, 117, 102, 55, 63, 97, 127, 119, 111, 62, 27, 124, 31, 15};   // with characters
long rand_digit_1;
long rand_digit_2;
long rand_digit_3;
long rand_digit_4;

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
    rand_digit_1 = random(10);
    rand_digit_2 = random(10);
    rand_digit_3 = random(10);
    rand_digit_4 = random(10);
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

    digitalWrite(DATA_PIN,LOW);
    digitalWrite(LATCH_PIN,LOW);
    digitalWrite(CLK_PIN,LOW);

    randomSeed(analogRead(0));
}

void loop() {
    generate_code();
}