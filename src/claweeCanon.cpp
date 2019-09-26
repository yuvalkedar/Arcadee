#include "claweeCanon.h"
#include "Arduino.h"

#define NUM_PIXELS (8)
#define CANON_MAX (95)
#define LAUNCHER_PIN (3)
#define UPDATE_MS (100)  // in milliseconds

claweeCanon::claweeCanon(uint16_t pixels, uint8_t pin, uint8_t type) : Adafruit_NeoPixel(pixels, pin, type) {
    increment = 1;
}

void claweeCanon::Restart() {
    analogWrite(LAUNCHER_PIN, CANON_MIN);
    led_bar = 0;
}

void claweeCanon::Update() {
    // NOTICE: Sometimes the led_bar is stuck on the first led and the strength goes maximum.
    if (millis() - last_update > UPDATE_MS * 3) {
        last_update = millis();
        setPixelColor(led_bar, led_bar_colour[led_bar]);
        show();
        led_bar += increment;
        if (led_bar <= 0 || led_bar >= 7) increment = -increment;
        if (led_bar <= 7) setPixelColor(led_bar + 1, 0x00);
        Serial.print(" led bar: ");
        Serial.println(led_bar);

        strength = map(led_bar, 0, NUM_PIXELS - 1, CANON_MIN, CANON_MAX);
        analogWrite(LAUNCHER_PIN, strength);
        Serial.print("Canon strength: ");
        Serial.print(strength);
    }
}
