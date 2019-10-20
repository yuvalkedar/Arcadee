#include "claweeServo.h"
#include "Arduino.h"

#define AIMING_SERVO_MIN (87)
#define AIMING_SERVO_MAX (127)
#define MAGAZINE_LOADING_POSITION (90)
#define MAGAZINE_RESTART_POSITION (180)

ClaweeServo::ClaweeServo(uint8_t interval) {
    servo_update = interval;
    increment = 1;
}

void ClaweeServo::Attach(uint8_t pin) {
    servo.attach(pin);
}

void ClaweeServo::Detach() {
    servo.detach();
}

void ClaweeServo::Restart() {
    position = AIMING_SERVO_MIN + ((AIMING_SERVO_MAX - AIMING_SERVO_MIN) / 2);
    servo.write(position);
}

void ClaweeServo::Magazine_restart() {
    servo.write(MAGAZINE_RESTART_POSITION);
}

void ClaweeServo::Load() {
    servo.write(MAGAZINE_LOADING_POSITION);
    // magazine_restart();
}

void ClaweeServo::Update() {  //TODO: Fix the canon that sometimes jumps to the center
    if (millis() - last_update > servo_update) {
        last_update = millis();
        position += increment;
        servo.write(position);
        Serial.print(F("servo position: "));
        Serial.println(position);
        if (position <= AIMING_SERVO_MIN || position >= AIMING_SERVO_MAX) increment = -increment;
    }
}
