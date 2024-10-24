#ifndef Update_hpp
#define Update_hpp

#include <Arduino.h>
#include <HTTPClient.h>
#include <ESP32httpUpdate.h> // This library works for updating, when the built in Update.h does not work with HTTPS URL's.
#include <WiFi.h>
#include <config.hpp>

extern String wifiSsid;
extern String wifiPassword;
extern String updateUrl;

class Update
{
private:
public:
    static void checkForUpdate();
};

#endif