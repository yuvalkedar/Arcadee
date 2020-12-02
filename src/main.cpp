#include <Arduino.h>
#include <Servo.h>

#define BTN_PIN (2)
#define SERVO_PIN (3)
#define WINNING_SENSOR_PIN (4)

#define SERVO_MIN (20)
#define SERVO_MAX (110)
#define RESET_MS  (2000)

enum {
    IDLE,
    WIN,
    WAIT,
    RESET
};

Servo door;

uint8_t game_state;
uint32_t door_ms;

void setup()
{
    pinMode(BTN_PIN, INPUT_PULLUP);  
    pinMode(SERVO_PIN, OUTPUT);
    pinMode(WINNING_SENSOR_PIN, OUTPUT);

    digitalWrite(WINNING_SENSOR_PIN, LOW);
    door.attach(SERVO_PIN);

    //clear balls
    door.write(SERVO_MIN);
    delay(2000);
    door.write(SERVO_MAX);
}

void loop()
{
    uint32_t cur_ms = millis();

    switch (game_state) {
        case IDLE:
            if (!digitalRead(BTN_PIN)) game_state = WIN;
            break;
        case WIN:
            door.write(SERVO_MIN);
            digitalWrite(WINNING_SENSOR_PIN, HIGH);
            door_ms = cur_ms;
            game_state = WAIT;
            break;
        case WAIT:
            if (cur_ms - door_ms >= RESET_MS) {
                door_ms = cur_ms;
                game_state = RESET;
            }
            break;
        case RESET:
            door.write(SERVO_MAX);
            digitalWrite(WINNING_SENSOR_PIN, LOW);
            game_state = IDLE;
            break;
    }
}