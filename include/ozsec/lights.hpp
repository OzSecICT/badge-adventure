#ifndef Lights_hpp
#define Lights_hpp
#include <Arduino.h>
#include <FastLED.h>

#ifdef BOARD_SIMON
// Set LED's for OzSec 2023 badge for testing
#define SIMPLE_NUM_LEDS 9 // Number of LEDs in the simple LED array, includes RGB eye as individual LED's
const int all_leds[SIMPLE_NUM_LEDS] = {GPIO_NUM_10, GPIO_NUM_14, GPIO_NUM_13, GPIO_NUM_12, GPIO_NUM_11, GPIO_NUM_15, GPIO_NUM_5, GPIO_NUM_4, GPIO_NUM_3};
#define STRIP_DATA_PIN 18
#else
// Set LED's for OzSec 2024 badge
#define SIMPLE_NUM_LEDS 9 // Number of LEDs in the simple LED array
const int all_leds[SIMPLE_NUM_LEDS] = {GPIO_NUM_17, GPIO_NUM_3, GPIO_NUM_46, GPIO_NUM_18, GPIO_NUM_8, GPIO_NUM_13, GPIO_NUM_12, GPIO_NUM_9, GPIO_NUM_11};
#define STRIP_DATA_PIN 10 // Data PIN for the RGB strip
#endif

#define STRIP_NUM_LEDS 1                 // Number of LEDs in the RGB strip
#define LED_MAX_BRIGHTNESS 10            // Maximum brightness for the simple LEDs. Set to 15 so they aren't super bright.
#define LED_FADE_DELAY 50                // Delay in ms for fading the simple LEDs off and on.
extern struct CRGB leds[STRIP_NUM_LEDS]; // Array to hold color data for the RGB strip LEDs
extern int ledTwinkleMode;

class Lights
{
private:
public:
    static void init();
    static void ledOn(int led, bool fade);
    static void ledOff(int led, bool fade);
    static void stripOn(CRGB color, int brightness);
    static void stripOff();
    static void stripBrightness(int brightness);
    static void heartbeat();
    static void twinkle();
    static int ledStatus[SIMPLE_NUM_LEDS];
    static int getLedStatus(int led);
    static void setLedStatus(int led, int status);
    static int getLedIndex(int led);
};

#endif