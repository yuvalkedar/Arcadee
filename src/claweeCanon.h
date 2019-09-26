#pragma once

#include <Adafruit_NeoPixel.h>

#define NUM_PIXELS (8)
#define CANON_MIN (55)

class claweeCanon : public Adafruit_NeoPixel {
   public:
    claweeCanon(uint16_t pixels, uint8_t pin, uint8_t type);

    void Restart();

    void Update();

   private:
    int strength = CANON_MIN;
    uint8_t increment;
    uint32_t last_update;

    uint8_t led_bar = 0;
    uint32_t led_bar_colour[NUM_PIXELS] = {0x00cc00, 0x00cc00, 0x66cc00, 0xcccc00, 0xff9900, 0xff6600, 0xff3300, 0xff0000};
};
