#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <Preferences.h>

extern Preferences preferences;

class OzSecBLE
{
private:
public:
    static void init();
    static void deinit();
    void loop();
    static bool scan();
};