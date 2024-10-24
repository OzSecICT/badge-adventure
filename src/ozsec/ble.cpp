#include <ozsec/ble.hpp>

// I don't really know how this works, it was copied from example code - rufflabs

const char *TAG = "BLE";

int scanTime = 5;            // In seconds, length of time to scan for BLE devices
int bleScanInterval = 30000; // In milliseconds, how often to scan for BLE devices
int lastScanTime = 0;        // Time of last ble scan
int lastModel2023FoundTime = 0;

bool bleInit;

BLEScan *pBLEScan;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        // Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
    }
};

/// @brief  Initialize the BLE device and set scan parameters
void OzSecBLE::init()
{
    if (bleInit == false)
    {
        ESP_LOGI(TAG, "Initializing.");

        BLEDevice::init("");
        pBLEScan = BLEDevice::getScan(); // create new scan
        pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
        pBLEScan->setActiveScan(true);
        pBLEScan->setInterval(100);
        pBLEScan->setWindow(99); // less or equal setInterval value
        bleInit = true;
    }

    return;
}

/// @brief  Deinitialize the BLE device and set scan parameters
void OzSecBLE::deinit()
{
    if (bleInit == true)
    {
        ESP_LOGI(TAG, "Deinitializing.");

        BLEDevice::deinit();
        bleInit = false;
    }

    return;
}

bool OzSecBLE::scan()
{
    if (bleInit == false)
    {
        OzSecBLE::init();
    }

    bool found = false;
    ESP_LOGI(TAG, "Scanning for Model 2023 Badge...");

    BLEScanResults foundDevices = pBLEScan->start(scanTime, true);
    int count = foundDevices.getCount();

    for (int i = 0; i < count; i++)
    {
        BLEAdvertisedDevice device = foundDevices.getDevice(i);
        String sensorName = device.getName().c_str();
        String address = device.getAddress().toString().c_str();
        int rssi = device.getRSSI();
        if (sensorName == "OzSec Model 2023 Badge BLE")
        {
            ESP_LOGI(TAG, "Found: %s %s %ddBm", sensorName.c_str(), address.c_str(), rssi);
            if (rssi > -50)
            {
                ESP_LOGI(TAG, "Badge %s registered at time %d", address.c_str(), lastModel2023FoundTime);
                found = true;
            }
        }
    }

    pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory

/*
 * The BLE documentation says that the BLEDevice::init()
 * returns a singleton. We shouldn't need to de-init it each time.
 * BUT - not de-init'ing means power consumption is almost double
 * after the BLE is initialized.
 */
    if (bleInit == true)
    {
        OzSecBLE::deinit();
    }

    return found;
}

/// @brief Scan for BLE devices and look for an 'OzSec Model 2023 Badge BLE' advertisement
void OzSecBLE::loop()
{
    // Don't scan if we already found a badge.
    if (!preferences.getBool("model2023found"))
    {
        // Only scan once every scan interval.
        if (millis() - lastScanTime < bleScanInterval)
        {
            return;
        }
        lastScanTime = millis();

        ESP_LOGI(TAG, "Scanning for Model 2023 Badge...");

        BLEScanResults foundDevices = pBLEScan->start(scanTime, true);
        int count = foundDevices.getCount();

        for (int i = 0; i < count; i++)
        {
            BLEAdvertisedDevice device = foundDevices.getDevice(i);
            String sensorName = device.getName().c_str();
            String address = device.getAddress().toString().c_str();
            int rssi = device.getRSSI();
            if (sensorName == "OzSec Model 2023 Badge BLE")
            {
                ESP_LOGI(TAG, "Found: %s %s %ddBm", sensorName.c_str(), address.c_str(), rssi);
                if (rssi > -50)
                {
                    if (preferences.getBool("model2023found") != true)
                        preferences.putBool("model2023found", true);

                    lastModel2023FoundTime = millis(); // Record the time when Model2023Found is set to true
                    preferences.putInt("model2023foundtime", lastModel2023FoundTime);

                    ESP_LOGI(TAG, "Badge %s registered at time %d", address.c_str(), lastModel2023FoundTime);
                }
            }
        }

        pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory

        // De-initialize if the badge was found
        if (preferences.getBool("model2023found"))
        {
            ESP_LOGI(TAG, "De-initializing BLE.");
            BLEDevice::deinit();
        }
    }
}