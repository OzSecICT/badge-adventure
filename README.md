# OzSec 2024 Adventure Badge

This repo contains the code and KiCad files to design the OzSec 2024 badge. 

## Theme

The theme of this year's badge is 'Light The Badge' where the front of the badge has LEDs representing various cities. In the default mode it will be twinkly and randomly blink the LEDs. 

When you connect to the badge via a serial console, a text-based adventure RPG will be playable. Each city in the game is represented by an LED, and each city will have a quest that needs to be completed. 

Once you complete each city's quest, that LED will light up. You need to light up every city in order to unlock access to Wichita and change the RGB LED. Completing that ultimate quest will reveal a CTF flag and change the RGB LED to green. 

## Codebase

The badge is based on an ESP32-S3 using the Arduino framework. This repo is designed to be opened in VS Code with the PlatformIO extension. 

### Files

Brief overview of the files and what they do:

**include/config.hpp:**
- Tracks the currently developed version for use with updates over the web. 
- WiFi SSID and Password defaults.

**include/ozsec/update.hpp and src/ozsec/update.cpp:**
- Manages over the air updates.
- Triggered by holding boot button in `main.cpp`

**includes/ozsec/ble.hpp and src/ozsec/ble.hpp:**
- Bluetooth Low Energy config, searches for an advertisement from an OzSec 2023: S1M0N badge. Sets a variable once a badge advertisement is found and stops searching.

**includes/ozsec/lights.hpp and src/ozsec/lights.cpp:**
- Manages the LEDs and NeoPixel RGB LED
- `Lights::twinkle()` is the main function that is called by `Adventure::bgloop()` on the second core in `main.cpp` to twinkle the lights when not in the game.

**src/main.cpp:**
- Main arduino `setup()` and `loop()` functions that call other class functions. 
- Sets up buttons using the OneButton library.
- Manages persistent data and game/badge states using the Preferences library.

**includes/ozsec/buttons.hpp:**
- Centralized setup of the buttons using OneButton

**includes/ozsec/rooms.hpp:**
- Room configs for the text based adventure

**includes/ozsec/npcs.hpp:**
- NPC config for the text based adventure

**includes/ozsec/adventure.hpp and src/ozsec/adventure.cpp:**
- The main text based adventure game.
- Manages character and badge states, what lights are lit, flags unlocked, etc.

### Wi-Fi setup
You can either set the wifi credentials in `config.hpp` or you can launch into the text game and enter `wifi` command to set it on your badge specifically.
