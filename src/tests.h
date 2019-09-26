#pragma once

#include "Arduino.h"

#define BASKET_SENSOR_PIN (A2)  // D16
#define BASKET_SENSOR_LIMIT (400)
#define LAUNCHER_PIN (3)

uint8_t strength = 80;

void check_basket_sensor() {
    Serial.println();
    if (analogRead(BASKET_SENSOR_PIN) > BASKET_SENSOR_LIMIT) {
        Serial.println(F("Congrats son, you might be the next Kobe Bryant!"));
    } else {
        Serial.println(F("FUCKKK"));
    }
}

void check_canon_strength() {
    uint8_t input = Serial.read();
    if (input == 'i') {  // increase strength
        strength = strength + 1;
    }
    if (input == 'd') {  // decrease strength
        strength = strength - 1;
    }

    analogWrite(LAUNCHER_PIN, strength);
    Serial.println(strength);
    delay(300);
}