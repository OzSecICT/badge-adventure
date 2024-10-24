#include <ozsec/lights.hpp>

struct CRGB leds[STRIP_NUM_LEDS];
int ledTwinkleMode;

// Array to keep track of the status of the map LEDs
int Lights::ledStatus[SIMPLE_NUM_LEDS];
bool ledFadingDirection[SIMPLE_NUM_LEDS];

/// @brief Setup the LED pins and RGB strip as needed
void Lights::init()
{
    // Setup simple LEDs
    for (int i = 0; i < SIMPLE_NUM_LEDS; i++)
    {
        pinMode(all_leds[i], OUTPUT);
    }

    // Setup RGB strip
    FastLED.addLeds<WS2812B, STRIP_DATA_PIN, GRB>(leds, STRIP_NUM_LEDS);
    FastLED.setBrightness(64);

    // Default ledStatus to false/off
    for (int i = 0; i < SIMPLE_NUM_LEDS; i++)
    {
        ledStatus[i] = 0;
        ledFadingDirection[i] = true;
    }

    ledTwinkleMode = 1;
}

// Get the known status of the specified LED pin
int Lights::getLedStatus(int led)
{
    int ledIndex;

    ledIndex = getLedIndex(led);
    if (ledIndex != -1)
    {
        return ledStatus[ledIndex];
    }
    else
    {
        return 0;
    }
}

// Function to set the LED status
void Lights::setLedStatus(int led, int status)
{
    int index = getLedIndex(led); // Get the index corresponding to the pin number
    if (index != -1)
    {
        ledStatus[index] = status; // Update the status
    }
}

/// @brief Get the index of specified LED pin
int Lights::getLedIndex(int led)
{
    for (int i = 0; i < SIMPLE_NUM_LEDS; i++)
    {
        if (all_leds[i] == led)
        {
            return i; // Return the index corresponding to the pin number
        }
    }
    return -1; // Return -1 if the pin is not found
}

/// @brief Turn on the specified LED pin
/// @param led
void Lights::ledOn(int led, bool fade = true)
{
    if (fade)
    {
        // If the LED is already on, don't do anything
        // Otherwise the LED would be turned off abruptly then faded back on
        if (getLedStatus(led))
            return;

        // Fade LED from off to max brightness
        for (int i = 0; i <= LED_MAX_BRIGHTNESS; i++)
        {
            analogWrite(led, i);
            delay(LED_FADE_DELAY);
        }
    }
    else
    {
        // Turn on the LED fully
        analogWrite(led, LED_MAX_BRIGHTNESS);
    }
    setLedStatus(led, true);
}

/// @brief Turn off the specified LED pin
/// @param led
void Lights::ledOff(int led, bool fade = true)
{
    if (fade)
    {
        // If the LED is already off, don't do anything
        // Otherwise the LED would be turned on fully then faded off
        if (getLedStatus(led) == false)
            return;

        // Fade LED from max brightness to off
        for (int i = LED_MAX_BRIGHTNESS; i >= 0; i--)
        {
            analogWrite(led, i);
            delay(LED_FADE_DELAY);
        }
    }
    else
    {
        // Turn off the LED fully
        analogWrite(led, 0);
    }
    setLedStatus(led, false);
}

/// @brief Turn on the RGB strip with a specific color
/// @param color
void Lights::stripOn(CRGB color, int brightness = 64)
{
    FastLED.setBrightness(brightness);
    leds[0] = color;
    FastLED.show();
}

/// @brief Turn off the RGB
void Lights::stripOff()
{
    leds[0] = CRGB(0, 0, 0);
    FastLED.show();
}

/// @brief Set the RGB brightness without changing the color
void Lights::stripBrightness(int brightness)
{
    FastLED.setBrightness(brightness);
    FastLED.show();
}

void Lights::heartbeat()
{
    int i, j;
    int ledArray[8] = {7,8,1,4,2,6,3,5};
    int rgbBrightness;
    //heartbeat
    for (i = 0; i < (64+32); i++)
    {
        Lights::stripBrightness(i);
        rgbBrightness = i;
        delay(1);
    }
    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 10; j++)
        {
            if (i == 0)
            {
                if (rgbBrightness > 64)
                {
                    rgbBrightness -= 3;
                    Lights::stripBrightness(rgbBrightness);
                }
            }
            analogWrite(all_leds[ledArray[i]], j);
            delay(1);
        }
    }
    // fade out
    for (i = rgbBrightness; i >= 0; i--)
    {
        Lights::stripBrightness(i);
        delay(1);
    }
    rgbBrightness = 0;
    for (i = 0; i < 8; i++)
    {
        for (j = 10; j >= 0; j--)
        {
            analogWrite(all_leds[ledArray[i]], j);
        }
    }
}

/// @brief Twinkle the front LEDs randomly
void Lights::twinkle()
{
    int led;
    int ledIndex;
    int ledStatus;

    switch (ledTwinkleMode)
    {
        case 2:
        {
            ledIndex = random(0, SIMPLE_NUM_LEDS) + 1;
            led = all_leds[ledIndex];
            ledStatus = getLedStatus(led);
            if (ledFadingDirection[ledIndex] == true)
            {
                if (ledStatus < LED_MAX_BRIGHTNESS)
                {
                    analogWrite(led, ledStatus+1);
                    setLedStatus(led, ledStatus+1);
                }
                else
                {
                    ledFadingDirection[ledIndex] = false;
                    delay(LED_FADE_DELAY);
                }
            }
            else
            {
                if (ledStatus > 0)
                {
                    analogWrite(led, ledStatus-1);
                    setLedStatus(led, ledStatus-1);
                }
                else
                {
                    ledFadingDirection[ledIndex] = true;
                    delay(LED_FADE_DELAY);
                }
            }
            break;
        }
        case 3:
        {
            Lights::heartbeat();
            delay(1);
            Lights::heartbeat();
            delay(500);
            break;
        }
        default:
        {
            for (ledIndex = 1; ledIndex <= SIMPLE_NUM_LEDS; ledIndex++)
            {
                // 1/4 chance of changing
                if (random(0, 4) == 1)
                {
                    led = all_leds[ledIndex];
                    if (getLedStatus(led))
                    {
                        ledOff(led);
                    }
                    else
                    {
                        ledOn(led);
                    }
                    delay(LED_FADE_DELAY);
                }
            }
            break;
        }
    }

    // Randomly change the color of the RGB strip
    // uint8_t red = random(256);
    // uint8_t green = random(256);
    // uint8_t blue = random(256);

    // leds[0] = CRGB(red, green, blue);
    // FastLED.setBrightness(5);
    // FastLED.show();
}