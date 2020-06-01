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
// uint8_t char_array[16] = {123, 96, 93, 117, 102, 55, 63, 97, 127, 119, 111, 62, 27, 124, 31, 15};   // with characters

void setup() {
    Serial.begin(115200);
    pinMode(DATA_PIN, OUTPUT);  
    pinMode(LATCH_PIN, OUTPUT);
    pinMode (CLK_PIN, OUTPUT);

    digitalWrite(DATA_PIN,LOW);
    digitalWrite(LATCH_PIN,LOW);
    digitalWrite(CLK_PIN,LOW);
}

void loop() {
    digitalWrite(LATCH_PIN,LOW);
    shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,char_array[6]);
    shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,char_array[9]);
    shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,char_array[9]);
    shiftOut(DATA_PIN,CLK_PIN,MSBFIRST,char_array[1]);
    digitalWrite(LATCH_PIN,HIGH);
    delay(5);
}