#include <ozsec/update.hpp>
#include <ozsec/lights.hpp>

String updateUrl = "https://raw.githubusercontent.com/OzSecICT/badge-adventure/refs/heads/main/firmware/"; // URL where firmware.bin can be found. Must end in '/'

/// @brief Checks for a firmware update and applies it if available.
/// Most of this was taken from the ESP32 HTTP Update example code.
void Update::checkForUpdate()
{
    HTTPClient httpClient;

    Serial.println("[Update] Checking for updates...");
    Lights::stripOn(CRGB::Green, 64);

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("[Update] WiFi not connected. Connecting...");
        WiFi.begin(wifiSsid, wifiPassword);

        // Only wait for 20 seconds before giving up.
        for (int i = 0; i < 20; i++)
        {
            if (WiFi.status() == WL_CONNECTED)
            {
                break;
            }
            delay(1000);
        }

        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("[Update] WiFi failed to connect. Restarting...");
            delay(5000);
            ESP.restart();
        }
    }

    Serial.println("[Update] WiFi connected.");
    Lights::stripOn(CRGB::Blue, 64);

    // Check version available
    httpClient.begin(updateUrl + "version");
    int httpCode = httpClient.GET();

    // @todo Test each of these cases. Currently HTTP_UPDATE_OK is known working.
    if (httpCode == 200)
    {
        int availableVersion = httpClient.getString().toInt();

        if (availableVersion > VERSION)
        {
            Serial.println("[Update] New firmware update available. Updating...");
            Lights::stripOn(CRGB::Purple, 64);
            ESPhttpUpdate.rebootOnUpdate(false); // Don't reboot after update, we reboot below after messages.
            t_httpUpdate_return updateStatus = ESPhttpUpdate.update(updateUrl + "firmware.bin");

            switch (updateStatus)
            {
            case HTTP_UPDATE_FAILED:
                Serial.printf("[Update] Update failed. Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
                Lights::stripOn(CRGB::Red, 64);
                break;
            case HTTP_UPDATE_NO_UPDATES:
                // @todo Setup OTA server that accepts x-ESP32-version and returns 304 if no update is available.
                Serial.println("[Update] No update available.");
                Lights::stripOn(CRGB::White, 64);
                break;
            case HTTP_UPDATE_OK:
                Serial.println("[Update] Firmware update completed.");
                Lights::stripOn(CRGB::Green, 64);
                break;
            }
        }
        else
        {
            Serial.println("[Update] No new firmware updates are available.");
            Lights::stripOn(CRGB::White, 64);
        }
    }
    else
    {
        Serial.println("[Update] Failed to connect to update server for version information.");
        Lights::stripOn(CRGB::Red, 64);
    }

    Serial.println("[Update] Restarting...");
    delay(5000);
    Lights::stripOff();
    ESP.restart();
}