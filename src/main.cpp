#include <Arduino.h>
#include <OneButton.h>
#include <Preferences.h>

#include <ozsec/adventure.hpp>
#include <ozsec/ble.hpp>
#include <ozsec/update.hpp>
#include <ozsec/lights.hpp>
#include <ozsec/buttons.hpp> // Setup buttons
#include <config.hpp>

Preferences preferences;
Adventure adventure;
OzSecBLE ozsecBLE;
extern bool bleInit;

String wifiSsid;
String wifiPassword;

// Setup background loop running on core 0, main loop() runs on core 1
TaskHandle_t BackgroundTask;              // Variable to hold the background task handle
void BackgroundTaskCode(void *parameter); // Function prototype for the background task

void setup()
{
    Serial.begin(115200);
    bleInit = false;
    Lights::init();
    preferences.begin("badge-state", false);

    // Set initial badge details. Not realy used at the moment, but maybe later.
    if (!(preferences.isKey("badge") && preferences.getString("badge") != "Adventure"))
    {
        preferences.putString("badge", "Adventure");
    }
    if (!(preferences.isKey("event") && preferences.getString("event") != "OzSec 2024"))
    {
        preferences.putString("event", "OzSec 2024");
    }
    if (!(preferences.isKey("version") && preferences.getString("version") != "Final-2.0"))
    {
        preferences.putString("version", "Final-2.0");
    }

    // Setup default wifi creds
    if (preferences.isKey("wifiSsid"))
    {
        wifiSsid = preferences.getString("wifiSsid");
    }
    else
    {
        wifiSsid = WIFI_SSID;
    }

    if (preferences.isKey("wifiPassword"))
    {
        wifiPassword = preferences.getString("wifiPassword");
    }
    else
    {
        wifiPassword = WIFI_PASSWORD;
    }

    // Print some badge info to serial
    Serial.printf("Badge: %s \r\nEvent: %s \r\nVersion: %s \r\n", preferences.getString("badge").c_str(), preferences.getString("event").c_str(), preferences.getString("version").c_str());
    preferences.end();
    // Setup OneButton functions to call when a button is pressed
    // Set up long press on BOOT to OTA
    boot_button.setPressMs(1500);
    boot_button.attachLongPressStart([]()
                                     {
        Serial.println("Starting OTA update...");
        Update::checkForUpdate(); }); // Call the OTA update function

    select_button.attachClick([]()
                              {
        adventure.processPromptResponse("select");; });
    boot_button.attachClick([]()
                            {
        adventure.processPromptResponse("boot");; });
    up_button.attachClick([]()
                          { adventure.processPromptResponse("n"); });
    right_button.attachClick([]()
                             { adventure.processPromptResponse("e"); });
    left_button.attachClick([]()
                            { adventure.processPromptResponse("w"); });
    down_button.attachClick([]()
                            { adventure.processPromptResponse("s"); });

    // Adventure initialization code
    adventure.init();

    // Background task stuff.
    xTaskCreatePinnedToCore(
        BackgroundTaskCode, /* Function to run the task */
        "BackgroundTask",   /* Name of the task */
        10000,              /* Stack size in words */
        NULL,               /* Task input parameter */
        0,                  /* Priority of the task */
        &BackgroundTask,    /* Task handle. */
        0);                 /* Core where the task should run */
}

void loop()
{

    // Run any relevant adventure loop code
    adventure.loop();

    // Button ticks to register presses. Part of the OneButton library.
    boot_button.tick();
    select_button.tick();
    up_button.tick();
    down_button.tick();
    left_button.tick();
    right_button.tick();
}

// This function runs on core 0 and is used to run background
// tasks to not interfer with the main loop code.
void BackgroundTaskCode(void *parameter)
{
    while (true)
    {
        // Run any relevant adventure loop code
        adventure.bgloop();
    }
}