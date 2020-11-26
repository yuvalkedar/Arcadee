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
#include <ShiftRegister74HC595.h>

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
#define WIN_SERVO_PIN (5)
#define BLOWER_PIN (6)
#define WINNING_SENSOR_PIN (7)  // winning switch pin in the RPi (GPIO12)
#define LATCH_PIN (9)
#define CLK_PIN (10)
#define DATA_PIN (11)
#define MOTOR_A_PIN (12)
#define MOTOR_B_PIN (13)

#define SENS_1_THRESHOLD (700)
#define SENS_2_THRESHOLD (600)
#define SENS_3_THRESHOLD (550)
#define SENS_4_THRESHOLD (650)
#define SENS_5_THRESHOLD (650)
#define SENS_6_THRESHOLD (620)
#define SENS_7_THRESHOLD (550)
#define SENS_8_THRESHOLD (650)
#define WIN_SERVO_MAX (10)
#define WIN_SERVO_MIN (130)
#define BALL_SERVO_MAX (180)    //OPEN POAITION
#define BALL_SERVO_MIN (100)     //CLOSE POSITION
#define BALL_DOOR_MS (1000)
#define BLOWER_MS (11000)
#define WIN_SERVO_DELAY_MS (150)
#define SENSORS_DEBOUNCE_MS (1)
#define DIGIT_DEBOUNCE_MS (2000)
#define SPINNING_MS (7000)

enum {
    DIGIT_1 = 0,
    DIGIT_2,
    DIGIT_3,
    DIGIT_4,
    DIGITS_COUNT,
    WAIT
};

enum {
    IDLE = 0,
    OPEN_BALL_DOOR,
    WAIT_CLOSE_DOOR,
    CLOSE_BALL_DOOR,
    WAIT_BLOWER_OFF,
    BLOWER_OFF,
};

Servo win_servo;
Servo ball_servo;
Button second_btn(SECOND_BTN_PIN);
ShiftRegister74HC595<DIGITS_COUNT> sr(DATA_PIN, CLK_PIN, LATCH_PIN);

uint8_t char_array[9] = {96, 93, 117, 102, 55, 63, 97, 127};   // without characters, zero (= 123), and nine (?=119)
// uint8_t char_array[9] = {249, 164, 176, 153, 146, 130, 248, 128};   // fixed for home setup
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
uint32_t cur_ms;
uint32_t read_ms;
uint32_t spin_ms;
uint32_t end_game_ms;
uint32_t digit_ms;
uint8_t code_state = DIGIT_1;
uint8_t prev_code_state = WAIT;
uint8_t game_state = IDLE;

uint8_t get_sensors_state()
{
    uint8_t btns_mask = 0;
    for (uint8_t i = 0; i < sizeof(btns_pins); i++) {
        if (analogRead(btns_pins[i]) <= sensor_threshold[i]){
            btns_mask |= 1 << i;
        }
    }
    // Serial.println(btns_mask);   // Serial prints: 0 = no sens, 1 = 1, 2 = 2, 4 = 3, 8 = 4, 16 = 5, 32 = 6, 64 = 7, 128 = 8
    return btns_mask;
}

void clear_screen()
{
    for (uint8_t i = 0; i <= 3; i++) digits_buff[i] = 0;
    sr.setAll(digits_buff);
}


void generate_code()
{
    for (uint8_t d = 60; d > 0; d -= 5) {
        for (uint8_t i = 0; i < 8; i++) {
            for (uint8_t b = 0; b <= 3; b++) digits_buff[b] = segment[i];
            sr.setAll(digits_buff);
            delay(d);
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

void write_nice()
{
    for (uint8_t i = 0; i <= 3; i++) digits_buff[i] = nice_array[i];
    sr.setAll(digits_buff);
}

void update_win_servo(bool dir)    //dir 1 = open, dir 0 = close
{
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

void game_setup()
{
    update_win_servo(0);    // closes the door
    generate_code();
    digitalWrite(WINNING_SENSOR_PIN, LOW);
}

void update_code(uint8_t mask)
{
    switch(code_state) {
        case DIGIT_1:
            digitalWrite(WINNING_SENSOR_PIN, LOW);
            if (mask == num_array[rand_digit_4]) {
                digits_buff[0] = 0;
                digits_buff[1] = char_array[rand_digit_3];
                digits_buff[2] = char_array[rand_digit_2];
                digits_buff[3] = char_array[rand_digit_1];
                sr.setAll(digits_buff);
                digit_ms = millis();
                code_state = WAIT;
            }
            break;
        case DIGIT_2:
            prev_code_state = DIGIT_1;
            digitalWrite(WINNING_SENSOR_PIN, LOW);
            if (mask == num_array[rand_digit_3]) {
                digits_buff[0] = 0;
                digits_buff[1] = 0;
                digits_buff[2] = char_array[rand_digit_2];
                digits_buff[3] = char_array[rand_digit_1];
                sr.setAll(digits_buff);
                digit_ms = millis();
                code_state = WAIT;
            }
            break;
        case DIGIT_3:
            prev_code_state = DIGIT_2;
            digitalWrite(WINNING_SENSOR_PIN, LOW);
            if (mask == num_array[rand_digit_2]) {
                digits_buff[0] = 0;
                digits_buff[1] = 0;
                digits_buff[2] = 0;
                digits_buff[3] = char_array[rand_digit_1];
                sr.setAll(digits_buff);
                digit_ms = millis();
                code_state = WAIT;
            }
            break;
        case DIGIT_4:
            if (mask == num_array[rand_digit_1]) {
                clear_screen();
                ball_servo.write(BALL_SERVO_MIN);
                update_win_servo(1);
                digitalWrite(WINNING_SENSOR_PIN, HIGH);
                write_nice();
                game_setup();
                prev_code_state = WAIT;
                code_state = DIGIT_1;
            } else
                digitalWrite(WINNING_SENSOR_PIN, LOW);
            break;
        case WAIT:
            if (cur_ms - digit_ms >= DIGIT_DEBOUNCE_MS) {
                switch(prev_code_state) {
                    case WAIT:
                        code_state = DIGIT_2;
                        break;
                    case DIGIT_1:
                        code_state = DIGIT_3;
                        break;
                    case DIGIT_2:
                        code_state = DIGIT_4;
                        break;
                }
            }
            break;
    }
}

void switch_dir()
{
    static bool direction = 0;

    if (!direction) {
        digitalWrite(MOTOR_A_PIN, HIGH);
        digitalWrite(MOTOR_B_PIN, LOW);
        direction = 1;
    } else {
        digitalWrite(MOTOR_A_PIN, LOW);
        digitalWrite(MOTOR_B_PIN, HIGH); 
        direction = 0;
    }
}

void end_game_sequence()
{
    switch (game_state) {
        case IDLE:
            if (second_btn.released()) game_state = OPEN_BALL_DOOR;
            break;
        case OPEN_BALL_DOOR:
            ball_servo.write(BALL_SERVO_MAX);   // drop the ball
            digitalWrite(BLOWER_PIN, LOW);  // blower on
            end_game_ms = millis();
            game_state = WAIT_CLOSE_DOOR;
            break;
        case WAIT_CLOSE_DOOR:
            if (cur_ms - end_game_ms >= BALL_DOOR_MS) game_state = CLOSE_BALL_DOOR;
            break;
        case CLOSE_BALL_DOOR:
            ball_servo.write(BALL_SERVO_MIN);   // close ball door
            end_game_ms = millis();
            game_state = WAIT_BLOWER_OFF;
            break;
        case WAIT_BLOWER_OFF:
            if (cur_ms - end_game_ms >= BLOWER_MS) game_state = BLOWER_OFF;
            break;
        case BLOWER_OFF:
            digitalWrite(BLOWER_PIN, HIGH); // blower off
            game_state = IDLE;
            break;
    }
}

void game_loop()
{
    cur_ms = millis();

    if (cur_ms - spin_ms >= SPINNING_MS) {
        switch_dir();
        spin_ms = cur_ms;
    }

    if (cur_ms - read_ms >= SENSORS_DEBOUNCE_MS) {
        update_code(get_sensors_state());
        read_ms = cur_ms;
    }

    end_game_sequence();
}

void setup()
{
    win_servo.attach(WIN_SERVO_PIN);
    win_servo.write(WIN_SERVO_MAX);  // restart win servo position

    ball_servo.attach(BALL_SERVO_PIN);
    ball_servo.write(BALL_SERVO_MIN);  // restart ball servo position

    //TODO: change to port manipulation
    // DDRB |= 0x3E;   // D9-D13 OUTPUT
    // DDRD |= 0xC0;   // D6-D7 OUTPUT
    // PORTD |= 0x44; // D2 INPUT_PULLUP, D6 OUTPUT_HIGH
    // PORTC = 0x00; // A0-A7 INPUT

    pinMode(DATA_PIN, OUTPUT);  
    pinMode(LATCH_PIN, OUTPUT);
    pinMode(CLK_PIN, OUTPUT);
    pinMode(WINNING_SENSOR_PIN, OUTPUT);
    pinMode(BLOWER_PIN, OUTPUT);
    pinMode(MOTOR_A_PIN, OUTPUT);
    pinMode(MOTOR_B_PIN, OUTPUT);

    for (uint8_t i = 0; i < sizeof(btns_pins); i++) pinMode(btns_pins[i], INPUT);
    pinMode(SECOND_BTN_PIN, INPUT_PULLUP);

    digitalWrite(WINNING_SENSOR_PIN, LOW);
    digitalWrite(BLOWER_PIN, HIGH); // blower off
    digitalWrite(DATA_PIN,LOW);
    digitalWrite(LATCH_PIN,LOW);
    digitalWrite(CLK_PIN,LOW);

    randomSeed(analogRead(0));

    generate_code();
}

void loop()
{
    game_loop();
}
