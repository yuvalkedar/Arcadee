#include <Arduino.h>

#include <Adafruit_NeoPixel.h> // https://github.com/adafruit/Adafruit_NeoPixel

#define CANON_MIN       (25)
#define CANON_MAX       (95)
#define LAUNCHER_PIN    (3)
#define NUM_PIXELS      (8)
#define LED_BAR_PIN     (A0)      // D14

Adafruit_NeoPixel ledBar(NUM_PIXELS, LED_BAR_PIN, NEO_GRB + NEO_KHZ800);

uint8_t increment = 1;
uint32_t last_update;

uint8_t led_bar = 0;
uint32_t led_bar_colour[] = {0x00cc00, 0x00cc00, 0x66cc00, 0xcccc00, 0xff9900, 0xff6600, 0xff3300, 0xff0000};

void setup() {
	Serial.begin(115200);
	pinMode(LAUNCHER_PIN, OUTPUT);
	ledBar.begin();
	ledBar.setBrightness(10);
	ledBar.clear();
	ledBar.show();
}

void loop() {
	if (millis() - last_update > 300) {
		last_update = millis();
		ledBar.setPixelColor(led_bar, led_bar_colour[led_bar]);
		ledBar.show();
		led_bar += increment;
		if (led_bar <= 0 || led_bar >= 7) increment = -increment;
		if (led_bar <= 7) ledBar.setPixelColor(led_bar+1, 0x00);
		Serial.print(" led bar: ");
		Serial.println(led_bar);
	}
}