#include <ozsec/adventure.hpp>
#include <ozsec/lights.hpp>
#include <ozsec/ble.hpp>

// Player and game state variables
CharacterState player;
GameState game;

// Callbacks are used so we don't block while waiting for text input.
Callback storedCallback;
PromptCallback promptCallback;
TalkCallback talkCallback;

// Track if the serial is connected, and if the prompt should be shown.
bool serialConnected;
bool showPrompt;

LightMode lightMode = TWINKLE;

String konamiStrings[10] = {"n", "n", "s", "s", "w", "e", "w", "e", "boot", "select"};
int konamiIndex;
#define KONAMI_INDEX_MAX 10

/// @brief Initialize the game state and load saved data.
void Adventure::init()
{
    // Load game data
    load();

    printHelp();

    // Default states
    serialConnected = false;
    showPrompt = true;

    // Set the initial callback to display the room
    setCallback(&Adventure::displayRoom);
}

/// @brief Main game loop
void Adventure::loop()
{
    // Print "Press enter" every 5 seconds until we know serial is connected
    static unsigned long lastPrint = 0;
    if (!serialConnected && millis() - lastPrint > 5000)
    {
        printHelp();
        lastPrint = millis();
    }

    if (!serialConnected && Serial.available() > 0)
    {
        // Serial is connected, clear the input buffer
        while (Serial.available() > 0)
        {
            Serial.read();
        }

        serialConnected = true;
        if (lightMode == TWINKLE)
        {
            lightMode = ADVENTURE;
        }
        Serial.println("Starting serial console...\n\n");
    }

    if (serialConnected)
    {
        // Show will display what is needed based on the callback set.
        show();

        // Handle anything that needs done in the background
        stateUpdate();

        // Prompt for input
        prompt();
    }
}

/// @brief This loop runs on core 0 along with the BLE scan.
/// This loop will be delayed periodically during BLE scans.
void Adventure::bgloop()
{
    // Handle LED mode
    switch (lightMode)
    {
    case TWINKLE:
        Lights::twinkle();
        if (ledTwinkleMode != 2)
        {
            if (game.qwichita)
            {
                Lights::stripOn(CRGB(0, 255, 0), 25);
            }
            else
            {
                Lights::stripOn(CRGB(255, 0, 0), 25);
            }
        }
        break;
    case ADVENTURE:
        ledMap();
        delay(1000);
        break;
    }
}

/// @brief Handles the LEDs when in ADVENTURE mode.
void Adventure::ledMap()
{
    if (game.qmodel2023)
    {
        // Turn on the LED
        digitalWrite(all_leds[0], HIGH);
    }
    else
    {
        // Turn off the LED
        digitalWrite(all_leds[0], LOW);
    }

    // Chanute
    if (game.qchanute)
    {
        Lights::ledOn(all_leds[1], false);
    }
    else
    {
        Lights::ledOff(all_leds[1], false);
    }

    // Pittsburgh
    if (game.qpittsburg)
    {
        Lights::ledOn(all_leds[2], false);
    }
    else
    {
        Lights::ledOff(all_leds[2], false);
    }

    // Kansas City
    if (game.qkansascity)
    {
        Lights::ledOn(all_leds[3], false);
    }
    else
    {
        Lights::ledOff(all_leds[3], false);
    }

    // Topeka
    if (game.qtopeka)
    {
        Lights::ledOn(all_leds[4], false);
    }
    else
    {
        Lights::ledOff(all_leds[4], false);
    }

    // Goodland
    if (game.qgoodland)
    {
        Lights::ledOn(all_leds[5], false);
    }
    else
    {
        Lights::ledOff(all_leds[5], false);
    }

    // Dodge City
    if (game.qdodgecity)
    {
        Lights::ledOn(all_leds[6], false);
    }
    else
    {
        Lights::ledOff(all_leds[6], false);
    }

    // Newton
    if (game.qnewton)
    {
        Lights::ledOn(all_leds[7], false);
    }
    else
    {
        Lights::ledOff(all_leds[7], false);
    }

    // Ellsworth
    if (game.qellsworth)
    {
        Lights::ledOn(all_leds[8], false);
    }
    else
    {
        Lights::ledOff(all_leds[8], false);
    }

    // Wichita
    if (game.qwichita)
    {
        // Turn on RGB strip
        Lights::stripOn(CRGB(0, 255, 0), 25);
    }
    else
    {
        // Keep RGP strip red
        Lights::stripOn(CRGB(255, 0, 0), 25);
    }
}

/// @brief Print help message to the serial console.
void Adventure::printHelp()
{
    Serial.println("\n\nHold BOOT to start OTA firmware update.");
    Serial.println("");
    Serial.println("Press enter to activate the serial console.");
}

/// @brief Load game and player state from memory.
void Adventure::load()
{
    // Keep in sync with Adventure::save()
    preferences.begin("game-data", false);
    // Load game state from memory, with default values if not set.
    game.playing = false;
    game.message = "";
    game.cheats = false;

    // Quest data
    game.qmodel2023 = preferences.getBool("qmodel2023", false);
    game.qtrainingvault = preferences.getBool("qtrainingvault", false);
    game.qtraining = preferences.getBool("qtraining", false);
    game.qchanute = preferences.getBool("qchanute", false);
    game.qpittsburg = preferences.getBool("qpittsburg", false);
    game.qkansascity = preferences.getBool("qkansascity", false);
    game.qtopeka = preferences.getBool("qtopeka", false);
    game.qgoodland = preferences.getBool("qgoodland", false);
    game.qdodgecity = preferences.getBool("qdodgecity", false);
    game.qnewton = preferences.getBool("qnewton", false);
    game.qellsworth = preferences.getBool("qellsworth", false);
    game.qwichita = preferences.getBool("qwichita", false);

    // Load player data
    player.name = preferences.getString("playername", "User");
    player.room = preferences.getInt("playerroom", TRAININGTENT);
    player.previousRoom = preferences.getInt("playerroom", TRAININGTENT);
    player.npc = -1; // -1 means no NPC is being talked to
    player.dialogIndex = 0;
    player.notebook = preferences.getString("playernotebook", "Your notebook has the following entries:\r\n\r\n");
    player.beacon = preferences.getInt("playerbeacon", 1);
    preferences.end();

    // Player inventory is handled by the game-data-inventory preference namespace
    preferences.begin("game-data-inventory", true);
    bool inventoryInit = preferences.isKey("INIT");
    if (inventoryInit == false)
    {
        preferences.end();
        preferences.begin("game-data-inventory", false);
        preferences.putBool("INIT", true);
        for (int i = 0; i < INVENTORY_ITEM_INDEX_COUNT; i++)
        {
            preferences.putInt(InventoryItems[i], 0);
            player.inventory[i] = 0;
        }
        preferences.end();
        preferences.begin("game-data-inventory", true);
        addItem(INVENTORY_ITEM_LANTERN);
        addItem(INVENTORY_ITEM_MAP);
    }

    konamiIndex = 0;
}

/// @brief Save game and player state to memory.
void Adventure::save()
{
    preferences.begin("game-data", false);

    // Save specific variables we care about only if they have changed
    if (preferences.getString("playername", "") != player.name)
    {
        preferences.putString("playername", player.name);
    }

    if (preferences.getInt("playerroom", TRAININGTENT) != player.room)
    {
        preferences.putInt("playerroom", player.room);
    }

    if (preferences.getString("playernotebook", "") != player.notebook)
    {
        preferences.putString("playernotebook", player.notebook);
    }

    if (preferences.getInt("playerbeacon", 1) != player.beacon)
    {
        preferences.putInt("playerbeacon", player.beacon);
    }

    // Quest data
    if (preferences.getBool("qmodel2023", false) != game.qmodel2023)
    {
        preferences.putBool("qmodel2023", game.qmodel2023);
    }

    if (preferences.getBool("qtrainingvault", false) != game.qtrainingvault)
    {
        preferences.putBool("qtrainingvault", game.qtrainingvault);
    }

    if (preferences.getBool("qtraining", false) != game.qtraining)
    {
        preferences.putBool("qtraining", game.qtraining);
    }

    if (preferences.getBool("qchanute", false) != game.qchanute)
    {
        preferences.putBool("qchanute", game.qchanute);
    }

    if (preferences.getBool("qpittsburg", false) != game.qpittsburg)
    {
        preferences.putBool("qpittsburg", game.qpittsburg);
    }

    if (preferences.getBool("qkansascity", false) != game.qkansascity)
    {
        preferences.putBool("qkansascity", game.qkansascity);
    }

    if (preferences.getBool("qtopeka", false) != game.qtopeka)
    {
        preferences.putBool("qtopeka", game.qtopeka);
    }

    if (preferences.getBool("qgoodland", false) != game.qgoodland)
    {
        preferences.putBool("qgoodland", game.qgoodland);
    }

    if (preferences.getBool("qdodgecity", false) != game.qdodgecity)
    {
        preferences.putBool("qdodgecity", game.qdodgecity);
    }

    if (preferences.getBool("qnewton", false) != game.qnewton)
    {
        preferences.putBool("qnewton", game.qnewton);
    }

    if (preferences.getBool("qellsworth", false) != game.qellsworth)
    {
        preferences.putBool("qellsworth", game.qellsworth);
    }

    if (preferences.getBool("qwichita", false) != game.qwichita)
    {
        preferences.putBool("qwichita", game.qwichita);
    }

    preferences.end();

    preferences.begin("game-data-inventory", false);
    for (int i = 0; i < INVENTORY_ITEM_INDEX_COUNT; i++)
    {
        preferences.putInt(InventoryItems[i], player.inventory[i]);
    }
    preferences.end();
}

/// @brief Check if the player has a specific item in their inventory.
/// @param item
/// @return bool
bool Adventure::hasItem(int item)
{
    bool retVal = false;

    if (item < INVENTORY_ITEM_INDEX_COUNT)
    {
        if (player.inventory[item] > 0)
        {
            retVal = true;
        }
    }

    return retVal;
}

/// @brief Add an item to player inventory
/// @param item
void Adventure::addItem(int item)
{
    if (item < INVENTORY_ITEM_INDEX_COUNT)
    {
        player.inventory[item]++;
    }

    return;
}

void Adventure::removeItem(int item)
{
    if (item < INVENTORY_ITEM_INDEX_COUNT)
    {
        if (player.inventory[item] > 0)
        {
            player.inventory[item]--;
        }
    }

    return;
}

// I don't think this is currently used...
void Adventure::setTalkCallback(TalkCallback callback)
{
    talkCallback = callback;
}

/// @brief Set the function to be called by Adventure::prompt()
/// @param callback
void Adventure::setPromptCallback(PromptCallback callback)
{
    promptCallback = callback;
}

/// @brief Sets the function to be called by Adventure::show()
/// @param callback
void Adventure::setCallback(Callback callback)
{
    storedCallback = callback;
}

// I don't think this is currently used...
void Adventure::unsetTalkCallback()
{
    talkCallback = NULL;
}

/// @brief Unset the function to be called by Adventure::prompt()
void Adventure::unsetPromptCallback()
{
    promptCallback = NULL;
}

/// @brief Unset the function to be called by Adventure::show()
void Adventure::unsetCallback()
{
    storedCallback = NULL;
}

void Adventure::completeTraining()
{
    game.qtraining = true;
    save();

    setCallback(&Adventure::displayMessage);
}

/// @brief Set the player's nickname
void Adventure::setNickname(String response)
{
    player.name = response;
    preferences.putString("playername", player.name);
    game.message = "Your nickname is now " + player.name + ".";
    setCallback(&Adventure::displayMessage);
    unsetPromptCallback();
}

/// @brief Confirms the wifi settings to be changed
/// @param response
void Adventure::confirmWifi(String response)
{
    if (response == String('y'))
    {
        game.message = "Please enter the new SSID: ";
        setCallback(&Adventure::displayMessage);
        setPromptCallback(&Adventure::setWifiSsid);
    }
    else
    {
        game.message = "WiFi settings not changed.";
        setCallback(&Adventure::displayMessage);
        unsetPromptCallback();
    }
}

/// @brief Set the WiFi SSID
void Adventure::setWifiSsid(String response)
{
    wifiSsid = response;
    preferences.putString("wifiSsid", wifiSsid);
    game.message = "WiFi SSID set to '" + response + "'\r\nPlease enter the password:";
    setCallback(&Adventure::displayMessage);
    setPromptCallback(&Adventure::setWifiPassword);
}

/// @brief Set the WiFi password
void Adventure::setWifiPassword(String response)
{
    wifiPassword = response;
    preferences.putString("wifiPassword", wifiPassword);
    game.message = "WiFi Password set.";
    setCallback(&Adventure::displayMessage);
    unsetPromptCallback();
}

/// @brief Handle room specific actions. This is the majority of the game world logic.
void Adventure::roomAction(String action)
{
    // Invalid directions will return -1
    if (player.room == -1)
    {
        player.room = player.previousRoom;
        game.message = "You can't go that way.";
        setCallback(&Adventure::displayMessage);
        return;
    }

    if (player.room < -1 || player.room >= sizeof(rooms) / sizeof(rooms[0]))
    {
        player.room = GRANDLOBBY;
        game.message = "You found yourself somewhere dark and hazy, after taking a few steps you are in the Grand Lobby.";
        setCallback(&Adventure::displayMessage);
        return;
    }

    player.previousRoom = player.room;

    // Handle Konami code special case first
    if (player.room == 497)
    {
        if (action == konamiStrings[konamiIndex])
        {
            konamiIndex++;
            if (konamiIndex == KONAMI_INDEX_MAX)
            {
                printFlag("KONAMI - OzSecCTF{K0n@m1_C0nTr@_Gr@d1u5}");
                konamiIndex = 0;
                setCallback(&Adventure::displayRoom);
            }
            return;
        }
        else
        {
            konamiIndex = 0;
        }
    }

    // Handle directions second
    if (action == "n" || action == "e" || action == "w" || action == "s")
    {
        if (action == "n")
        {
            player.room = rooms[player.room].neighbors[NORTH];
            setCallback(&Adventure::displayRoom);
            return;
        }
        else if (action == "e")
        {
            player.room = rooms[player.room].neighbors[EAST];
            setCallback(&Adventure::displayRoom);
            return;
        }
        else if (action == "w")
        {
            player.room = rooms[player.room].neighbors[WEST];
            setCallback(&Adventure::displayRoom);
            return;
        }
        else if (action == "s")
        {
            player.room = rooms[player.room].neighbors[SOUTH];
            setCallback(&Adventure::displayRoom);
            return;
        }
    }

    // Handle other specific room actions.
    for (int i = 0; i < MAX_ROOM_OPTIONS; i++)
    {
        if (action == rooms[player.room].options[i])
        {
            switch (player.room)
            {
            // BEGIN Training Area actions
            case THEVAULT:
                if (action == "flag")
                {
                    printFlag("OzSecCTF{V4u1t_hunt3r_h@cke2}");
                    game.message = "You inspect the pennant and find a CTF flag written on the back.";
                    setCallback(&Adventure::displayMessage);
                }
                else if (action == "talk")
                {
                    player.npc = 0; // ID of NPC we are talking to
                    setCallback(&Adventure::displayDialog);
                    setPromptCallback(&Adventure::talkToNPC); // Sets handler for responses
                }
                break;

            case GRANDLOBBY:
                if (action == "unlock")
                {
                    // Check that the key is available
                    if (hasItem(INVENTORY_ITEM_RED_KEYCARD))
                    {
                        game.qtrainingvault = true;
                        save();
                        game.message = "You swipe the Red Keycard against the reader and the door beeps and a thunk can be heard as the door unlocks and swings open.";
                        setCallback(&Adventure::displayMessage);
                        return;
                    }
                    else
                    {
                        game.message = "You don't have any items that would work with this lock.";
                        setCallback(&Adventure::displayMessage);
                        return;
                    }
                }
                break;

            case GREATOUTDOORS:
                if (action == "key")
                {
                    addItem(INVENTORY_ITEM_RED_KEYCARD);
                    game.message = "You pick up the battered Red Keycard.";
                    setCallback(&Adventure::displayMessage);
                }
                else if (action == "talk")
                {
                    player.npc = 1; // ID of NPC we are talking to
                    setCallback(&Adventure::displayDialog);
                    setPromptCallback(&Adventure::talkToNPC); // Sets handler for responses
                }
                else if (action == "button")
                {
                    if (game.qtraining)
                    {
                        Serial.println("You press the button embedded in the tree and feel a strange sensation as you are teleported away from the training area.");
                        player.room = GAMESTART;
                        setCallback(&Adventure::displayRoom);
                    }
                    else
                    {
                        Serial.println("You press the button, it makes a click, but nothing else happens.");
                        setCallback(&Adventure::displayRoom);
                    }
                }
                break;

            case TRAININGTENT:
                if (action == "paper")
                {
                    game.message = "The paper reads:\r\n\r\nWelcome to the text based adventure game!\r\n\r\nFirst off, you can type 'help' to get a list of all available commands. When you first enter a room, or when you 'look' you will see the title of the room, the description, and any actions in brackets (like [paper]) that you can type to interact with that object. Under that will be the available directions to go to a different room.\r\n\r\nTo move around, type the direction you want to go (n, e, w, s) if that direction is available.\r\n\r\nTo talk to NPCs, type 'talk'.\r\n\r\nTo see your inventory, type 'inventory' or 'i'.\r\n\r\nTo save your game, type 'save'.\r\n\r\nTo exit the game, type 'exit'.\r\n\r\nRemember to check out 'help' as there are more commands available.\r\n\r\nNext, let's try 'look'ing around, and then try talking to the janitor. When you're done, head out of the tent to the west to explore the world.";
                    setCallback(&Adventure::displayMessage);
                }
                else if (action == "talk")
                {
                    player.npc = 0; // ID of NPC we are talking to
                    setCallback(&Adventure::displayDialog);
                    setPromptCallback(&Adventure::talkToNPC); // Sets handler for responses
                }
                break;
            // END Training Area actions
            // BEGIN Kansas City actions
            case 11:
                if (action == "talk")
                {
                    game.message = "Driver: Oh, you aren't supposed to be here. Go see the maintenance manager... and don't mention I brought you here.";
                    setCallback(&Adventure::displayMessage);
                }
                else if (action == "load")
                {
                    game.qkcbus1024 = true;
                    game.message = "You take one of the blank tapes and insert them into the tape drive.";
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 14:
                if (action == "filter")
                {
                    game.qkcbus1138 = true;
                    game.message = "You remove the filter and replace it the one sitting on the seat.";
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 16:
                if (action == "tire")
                {
                    game.qkcbus2018 = true;
                    game.message = "You check the spare tire pressure, it is good.";
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 19:
                if (action == "talk")
                {
                    player.npc = 13;
                    setCallback(&Adventure::displayDialog);
                    setPromptCallback(&Adventure::talkToNPC);
                }
                break;

            case 20:
                if (action == "cheese")
                {
                    printFlag("OzSecCTF{Ch33s3_1s_L1f3}");
                    game.message = "You pick up the slice of cheese, and see a flag printed on the back.";
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 21:
                if (action == "bus")
                {
                    if (game.qkcbus1024 && game.qkcbus1138 && game.qkcbus2018 && hasItem(INVENTORY_ITEM_A_TICKET_TO_TOPEKA))
                    {
                        printFlag("OzSecCTF{Adv3nture_T1m3_1s_H3r3}");
                        game.qkansascity = true;
                        Serial.println("You board the bus and take off to Topeka");
                        sleep(2);
                        player.room = 61;
                        setCallback(&Adventure::displayRoom);
                    }
                    else
                    {
                        game.message = "You need approval from the maintenance manager before you can leave.";
                        setCallback(&Adventure::displayMessage);
                    }
                }
                break;
            // END Kansas City actions
            // BEGIN Topeka
            case 79:
                if (action == "drive")
                {
                    game.qtpkdrive1 = true;
                    game.message = "You pull the drive from the PC.";
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 81:
                if (action == "donuts")
                {
                    if (hasItem(INVENTORY_ITEM_BOX_OF_DONUTS))
                    {
                        printFlag("OzSecCTF{D0nut5_4r3_d3lici0us}");
                        game.message = "Everyone cheers as you hand out the donuts. You find a flag written on the bottom of the box.";
                        setCallback(&Adventure::displayMessage);
                    }
                    else
                    {
                        game.message = "If only you had a new box of donuts to share.";
                        setCallback(&Adventure::displayMessage);
                    }
                }
                else if (action == "analyze")
                {
                    if (game.qtpkdrive1 && game.qtpkdrive2 && game.qtpkdrive3 && game.qtpkdrive4 && game.qtpkdrive5)
                    {
                        printFlag("OzSecCTF{4n@lyz3_Th3_D@t@}");
                        game.qtopeka = true;
                        game.message = "The technicians analyze the drives and identify the source of the malware. They are able to swiftly disable it and restore operations.";
                        setCallback(&Adventure::displayMessage);
                    }
                    else
                    {
                        game.message = "Tech: We need all of the encrypted drives from inside the hospital to analyze and figure out this attack.";
                        setCallback(&Adventure::displayMessage);
                    }
                }
                break;

            case 86:
                if (action == "drive")
                {
                    game.qtpkdrive2 = true;
                    game.message = "You pull the drive from the PC.";
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 87:
                if (action == "donuts")
                {
                    addItem(INVENTORY_ITEM_BOX_OF_DONUTS);
                    game.message = "You pick up the box of donuts.";
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 89:
                if (action == "drive")
                {
                    game.qtpkdrive3 = true;
                    game.message = "You pull the drive from the PC.";
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 90:
                if (action == "drive")
                {
                    game.qtpkdrive4 = true;
                    game.message = "You pull the drive from the PC.";
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 93:
                if (action == "pi")
                {
                    addItem(INVENTORY_ITEM_A_RASPBERRY_PI);
                    game.message = "You pick up the Raspberry Pi. Maybe someone would be interested in it?";
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 94:
                if (action == "drive")
                {
                    game.qtpkdrive5 = true;
                    game.message = "You pull the drive from the PC.";
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 98:
                if (action == "pi")
                {
                    if (hasItem(INVENTORY_ITEM_A_RASPBERRY_PI))
                    {
                        printFlag("OzSecCTF{R@spb3rry_Pi}");
                        removeItem(INVENTORY_ITEM_A_RASPBERRY_PI);
                        game.message = "You hand the Raspberry Pi to the manager and they thank you. You find a flag written on the back of the Pi.";
                        setCallback(&Adventure::displayMessage);
                    }
                    else
                    {
                        game.message = "If only you had a Raspberry Pi to add to their collection.";
                        setCallback(&Adventure::displayMessage);
                    }
                }
                break;
            // END Topeka
            // BEGIN Chanute actions
            case 100:
                if (action == "poster")
                {
                    game.message = "SEEKING INFORMATION\n====================\nLocal reporter seeks information on the history of Chanute.\n\nFor more information visit Town Hall.\n\n- Chanute Tribune";
                    setCallback(&Adventure::displayMessage);
                }
                break;
            case 103:
                if (action == "coupon")
                {
                    if (hasItem(INVENTORY_ITEM_COFFEE_CONNECTION_BOGO_COUPON))
                    {
                        game.message = "You hand the coupon and some cash to the barista and they hand you two cups of coffee.";
                        addItem(INVENTORY_ITEM_TWO_CUP_O_JOES);
                        setCallback(&Adventure::displayMessage);
                    }
                    else
                    {
                        game.message = "You don't seem to have a valid coupon. The barista mentions that they put flyers around the town with the coupons if you can find one.";
                        setCallback(&Adventure::displayMessage);
                    }
                }
                else if (action == "beacon")
                {
                    player.beacon = player.room;
                    game.message = "You set your beacon to this location.\r\nYou can return here at any time by typing 'beacon'";
                    setCallback(&Adventure::displayMessage);
                }
                break;
            case 118:
                if (action == "talk")
                {
                    player.npc = 2; // ID of NPC we are talking to
                    setCallback(&Adventure::displayDialog);
                    setPromptCallback(&Adventure::talkToNPC); // Sets handler for responses
                }
                break;
            case 122:
                if (action == "sign")
                {
                    addItem(INVENTORY_ITEM_CHANOOGLE_SIGN);
                    save(); // Save the flag and quest state
                    game.message = "There is a rusty sign that reads 'Chanoogle' on it. This must be what Chanute was once named.\r\nAs you clear some dust off the sign it falls off, so you pick it up.";
                    setCallback(&Adventure::displayMessage);
                }
                break;
            case 127:
                if (action == "sign")
                {
                    game.message = "Someone has printed out a sign describes the latest coffees available at Coffee Connection.\nIt has a tear-off [coupon], with only a couple left.\nThe coupon is for buy one get one.";
                    setCallback(&Adventure::displayMessage);
                }
                else if (action == "coupon")
                {
                    addItem(INVENTORY_ITEM_COFFEE_CONNECTION_BOGO_COUPON);
                    game.message = "You tear off a coupon from the sign.";
                    setCallback(&Adventure::displayMessage);
                }
                break;
            case 130:
                if (action == "sign")
                {
                    game.message = "A sign reads 'Seeking information on Chanute history. Visit Town Hall for more information.' and is signed by the Chanute Tribune.";
                    setCallback(&Adventure::displayMessage);
                }
                break;
            case 137:
                if (action == "leaflet")
                {
                    addItem(INVENTORY_ITEM_CHANUTE_TRIBUNE_LEAFLET);
                    game.message = "You pick up a leaflet from the ground. It reads 'Chanute Tribune - Local News'\nOn the back side it has a request for information on the history of Chanute and mentions a reporter stationed at Town Hall.";
                    setCallback(&Adventure::displayMessage);
                }
                // END Chanute actions

                // BEGIN Pittsburg actions

            case 150:
                if (action == "billboard")
                {
                    game.message = "The digital sign appears to have been compromised by ransomware of some kind. It displays a crypto wallet address.";
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 153:
                if (action == "rose")
                {
                    addItem(INVENTORY_ITEM_A_RED_ROSE);
                    game.message = "The shopkeeper offers you the rose.\r\nYou pick up the rose and it smells sweet.";
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 157:
                if (action == "coffee")
                {
                    addItem(INVENTORY_ITEM_STACEYS_CUP_OF_COFFEE);
                    game.message = "You grab the coffee, maybe you'll run into Stacey later.";
                    setCallback(&Adventure::displayMessage);
                }
                else if (action == "beacon")
                {
                    player.beacon = player.room;
                    game.message = "You set your beacon to this location.\r\nYou can return here at any time by typing 'beacon'";
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 161:
                if (action == "letter")
                {
                    addItem(INVENTORY_ITEM_A_LETTER_TO_THE_PITTSBURG_COURTS);
                    game.message = "You pick up the letter.";
                    setCallback(&Adventure::displayMessage);
                }
                else if (action == "talk")
                {
                    player.npc = 6;
                    setCallback(&Adventure::displayDialog);
                    setPromptCallback(&Adventure::talkToNPC);
                }
                break;

            case 170:
                if (action == "talk")
                {
                    player.npc = 7;
                    setCallback(&Adventure::displayDialog);
                    setPromptCallback(&Adventure::talkToNPC);
                }
                break;

            case 171:
                if (action == "talk")
                {
                    player.npc = 8;
                    setCallback(&Adventure::displayDialog);
                    setPromptCallback(&Adventure::talkToNPC);
                }
                break;

            case 174:
                if (action == "drive")
                {
                    if (hasItem(INVENTORY_ITEM_A_USB_DRIVE_FROM_THE_PITTSBURG_PARK))
                    {
                        printFlag("OzSecCTF{Gray_H@t_H@ck3r}");
                        game.message = "You plug the USB drive you found in the park into the PC.\r\nThis doesn't feel right...";
                    }
                    else
                    {
                        game.message = "You don't have a USB drive.";
                    }
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 178:
                if (action == "drive")
                {
                    addItem(INVENTORY_ITEM_A_USB_DRIVE_FROM_THE_PITTSBURG_PARK);
                    game.message = "You pick up the USB drive and place it in your pocket.";
                }
                else if (action == "plant")
                {
                    if (hasItem(INVENTORY_ITEM_A_RED_ROSE))
                    {
                        printFlag("OzSecCTF{R0s3s_4r3_r3d}");
                        game.message = "You place the rose in the planter and it looks nice.";
                    }
                    else
                    {
                        game.message = "You don't have anything to plant.";
                    }
                }
                setCallback(&Adventure::displayMessage);
                break;

            case 193:
                if (action == "talk")
                {
                    player.npc = 9;
                    setCallback(&Adventure::displayDialog);
                    setPromptCallback(&Adventure::talkToNPC);
                }
                break;

            case 195:
                if (action == "ladder")
                {
                    addItem(INVENTORY_ITEM_A_LADDER);
                    game.message = "You pick up the ladder and carry it with you.";
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 198:
                if (action == "climb")
                {
                    if (hasItem(INVENTORY_ITEM_A_LADDER))
                    {
                        Serial.println("You prop the ladder against the eastern wall and climb up to the top.");
                        sleep(2);
                        for (int j = 0; j < 5; j++)
                        {
                            for (int k = 0; k < 2; k++)
                            {
                                Serial.println("||   ||");
                                usleep(5 * 1000 * 1000 / 25);
                            }
                            Serial.println("||===||");
                            usleep(5 * 1000 * 1000 / 25);
                            for (int k = 0; k < 2; k++)
                            {
                                Serial.println("||   ||");
                                usleep(5 * 1000 * 1000 / 25);
                            }
                        }
                        Serial.println("As you slide the ceiling tile out of the way, you can see the computer lab on the other side.");
                        sleep(2);
                        for (int j = 0; j < 5; j++)
                        {
                            for (int k = 0; k < 2; k++)
                            {
                                Serial.println("||   ||");
                                usleep(5 * 1000 * 1000 / 25);
                            }
                            Serial.println("||===||");
                            usleep(5 * 1000 * 1000 / 25);
                            for (int k = 0; k < 2; k++)
                            {
                                Serial.println("||   ||");
                                usleep(5 * 1000 * 1000 / 25);
                            }
                        }
                        Serial.println("You climb through the ceiling and over the wall, landing in the computer lab.");
                        sleep(2);
                        for (int j = 0; j < 5; j++)
                        {
                            for (int k = 0; k < 2; k++)
                            {
                                Serial.println("||   ||");
                                usleep(5 * 1000 * 1000 / 25);
                            }
                            Serial.println("||===||");
                            usleep(5 * 1000 * 1000 / 25);
                            for (int k = 0; k < 2; k++)
                            {
                                Serial.println("||   ||");
                                usleep(5 * 1000 * 1000 / 25);
                            }
                        }
                        player.room = 199;
                        setCallback(&Adventure::displayRoom);
                    }
                    else
                    {
                        game.message = "You don't have a ladder.";
                        setCallback(&Adventure::displayMessage);
                    }
                }
                break;

            case 199:
                if (action == "shutdown")
                {
                    game.qptsshutdown = true;
                    game.message = "You shut down the laptop and appear to stop the cyber attack against Pittsburg for now.";
                }
                else if (action == "unplug")
                {
                    if (game.qptsshutdown)
                    {
                        game.qptsunplug = true;
                        game.message = "You unplug the cables between the switches, preventing any further attacks from this location.";
                    }
                    else
                    {
                        game.message = "On second thought, you should shut down this laptop first.";
                    }
                }
                setCallback(&Adventure::displayMessage);
                break;
                // END Pittsburg actions

                // BEGIN Newton actions

            case 206: // pickup usb drive
                if (action == "pickup")
                {
                    addItem(INVENTORY_ITEM_PARKING_LOT_DRIVE);
                    game.message = "You pick up the USB drive.";
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 213: // games
                if (action == "playball")
                {
                    if (game.qnwtdisc)
                    {
                        printFlag("OzSecCTF{mu1t1_sp0rt_@thl3t3}");
                    }
                    game.qnwtball = true;
                    game.message = "You enjoy a quick game of baseball.";
                    setCallback(&Adventure::displayMessage);
                }
                else if (action == "throwdiscs")
                {
                    if (game.qnwtball)
                    {
                        printFlag("OzSecCTF{mu1t1_sp0rt_@thl3t3}");
                    }
                    game.qnwtdisc = true;
                    game.message = "You enjoy a quick game of disc golf.";
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 216: // talk to principal
                if (action == "talk")
                {
                    player.npc = 14;
                    setCallback(&Adventure::displayDialog);
                    setPromptCallback(&Adventure::talkToNPC);
                }
                break;

            case 221: // talk to it specialist
                if (action == "talk")
                {
                    player.npc = 15;
                    setCallback(&Adventure::displayDialog);
                    setPromptCallback(&Adventure::talkToNPC);
                }
                else if (action == "lunch")
                {
                    if (hasItem(INVENTORY_ITEM_NEWTON_LUNCH))
                    {
                        printFlag("OzSecCTF{Lunch_T1m3}");
                        game.message = "You hand the IT specialist the lunch and they thank you.";
                        removeItem(INVENTORY_ITEM_NEWTON_LUNCH);
                        setCallback(&Adventure::displayMessage);
                    }
                    else
                    {
                        game.message = "You don't have a lunch to give.";
                        setCallback(&Adventure::displayMessage);
                    }
                }

                break;

            case 222: // insert drive
                if (action == "insert")
                {
                    printFlag("OzSecCTF{b@d_@ct0r}");
                    game.message = "You insert the USB drive you found in the parking lot. Was this a good idea?";
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 225: // recovery page 3
                if (action == "pickup")
                {
                    addItem(INVENTORY_ITEM_NEWTON_RECOVERY_PAGE_3);
                    game.message = "You pick up the page.";
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 229: // page 2
                if (action == "pickup")
                {
                    addItem(INVENTORY_ITEM_NEWTON_RECOVERY_PAGE_2);
                    game.message = "You pick up the page.";
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 237: // page 1
                if (action == "pickup")
                {
                    addItem(INVENTORY_ITEM_NEWTON_RECOVERY_PAGE_1);
                    game.message = "You pick up the page.";
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 240: // lunch
                if (action == "pickup")
                {
                    addItem(INVENTORY_ITEM_NEWTON_LUNCH);
                    game.message = "You pick up the lunch.";
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 241: // page 4
                if (action == "pickup")
                {
                    addItem(INVENTORY_ITEM_NEWTON_RECOVERY_PAGE_4);
                    game.message = "You pick up the page.";
                    setCallback(&Adventure::displayMessage);
                }
                break;

                // END Newton actions

                // BEGIN Ellsworth actions
            case 257:
                if (action == "keycard")
                {
                    addItem(INVENTORY_ITEM_ELLSWORTH_WATER_TREATMENT_KEYCARD);
                    game.message = "You pick up the keycard.";
                    setCallback(&Adventure::displayMessage);
                }
                break;
            case 265:
                if (action == "checkin")
                {
                    if (hasItem(INVENTORY_ITEM_ELLSWORTH_WATER_TREATMENT_KEYCARD))
                    {
                        game.qelaccess = true;
                        game.message = "You show the guard your keycard and they wave you on.";
                        setCallback(&Adventure::displayMessage);
                    }
                    else
                    {
                        game.message = "You don't have the keycard needed to gain access. Perhaps the water plant office on Douglas could help.";
                        setCallback(&Adventure::displayMessage);
                    }
                }
                break;

            case 271:
                if (action == "soda")
                {
                    if (hasItem(INVENTORY_ITEM_A_SODA))
                    {
                        printFlag("OzSecCTF{S0d@_P0p_S3ns8tion}");
                        game.message = "The groundskeeper thanks you as he enjoys the soda.";
                        removeItem(INVENTORY_ITEM_A_SODA);
                        setCallback(&Adventure::displayMessage);
                    }
                    else
                    {
                        game.message = "You don't have a soda to give, maybe you could get one from a local fuel stop.";
                        setCallback(&Adventure::displayMessage);
                    }
                }
                break;

            case 279:
                if (action == "soda")
                {
                    addItem(INVENTORY_ITEM_A_SODA);
                    game.message = "You pick up a soda.";
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 285:
                if (action == "pobox")
                {
                    printFlag("OzSecCTF{P0st@l_S3rv1c3}");
                    game.message = "You open the PO Box and find a postcard inside with a flag written on the back.";
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 291:
                if (action == "talk")
                {
                    game.message = "Receptionist: We are all a bit busy trying to figure out why our water doesn't taste right. If you can help, check with the lead technician in the back.";
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 294:
                if (action == "talk")
                {
                    game.message = "Plant Manager: Hello there. I'm a bit busy at the moment, but if you can help, check with the tech over in the control room.";
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 296:
                if (action == "talk")
                {
                    if (game.qellevels)
                    {
                        game.qellsworth = true;
                        printFlag("OzSecCTF{W@t3r_F1ltr@t10n}");
                        game.message = "Tech: That fixed it! Our water is back to normal!";
                    }
                    else
                    {
                        game.message = "Tech: I don't know why the water is tasting off, but it's not good. Can you check the levels in the Quality Control room?";
                    }
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 297:
                if (action == "check")
                {
                    if (game.qellevels)
                    {
                        game.message = "The water tastes normal and the levels are good. Check with the lead technician next.";
                        setCallback(&Adventure::displayMessage);
                    }
                    else
                    {
                        game.message = "The water tastes wrong, and the levels are off.";
                        setCallback(&Adventure::displayMessage);
                    }
                }
                break;

            case 298:
                if (action == "restore")
                {
                    if (game.qellaptop)
                    {
                        game.qellevels = true;
                        game.message = "You restore the levels to normal. Check the quality of the water next.";
                        setCallback(&Adventure::displayMessage);
                    }
                    else
                    {
                        game.message = "As soon as the levels are restored, they go out of wack again immediately. Something else is changing them.";
                        setCallback(&Adventure::displayMessage);
                    }
                }
                break;

            case 299:
                if (action == "unplug")
                {
                    game.qellaptop = true;
                    game.message = "You unplug the laptop and appear to stop the cyber attack against Ellsworth for now.";
                    setCallback(&Adventure::displayMessage);
                }
                break;
                // END Ellsworth actions

                // BEGIN Goodland actions
            case 302:
                if (action == "alert")
                {
                    game.message = "You listen in to the alert and hear that this taxi requires repair from the local auto repair shop.\r\nPerhaps you could find a way to get the taxi repaired.";
                    setCallback(&Adventure::displayMessage);
                }
                else if (action == "taxi")
                {
                    if (hasItem(INVENTORY_ITEM_A_GOODLAND_TAXI_SERVICE_KEY))
                    {
                        if (hasItem(INVENTORY_ITEM_A_GTS_FIRMWARE_DRIVE))
                        {
                            game.qgts1 = true;
                            game.message = "You unlock the taxi, and insert the firmware update drive.\r\nAfter a few minutes the taxi reboots and is ready for service.";
                            setCallback(&Adventure::displayMessage);
                        }
                        else
                        {
                            game.message = "You don't have the firmware drive to repair the taxi.\r\nPerhaps you should find the company supporting these robot taxis.";
                            setCallback(&Adventure::displayMessage);
                        }
                    }
                    else
                    {
                        game.message = "You don't have a key to access the interior of the taxi.\r\nThere may be an auto repair shop somewhere in town.";
                        setCallback(&Adventure::displayMessage);
                    }
                }
                break;
            case 304:
                if (action == "painting")
                {
                    printFlag("OzSecCTF{V1nc3nt_van_g00dl@nd}");
                    game.message = "This is a very large rendition of one of Vincent van Gogh's fameous paintings.\r\nIt's a beautiful piece of art.";
                    setCallback(&Adventure::displayMessage);
                }
                break;
            case 317:
                if (action == "ticket")
                {
                    addItem(INVENTORY_ITEM_A_PLANE_TICKET_FROM_GOODLAND_MUNICPAL_AIRPORT);
                    printFlag("OzSecCTF{f1y0ver}");
                    game.message = "You pick up the plane ticket, it's expired.";
                    setCallback(&Adventure::displayMessage);
                }
                else
                {
                    if (action == "taxi")
                    {
                        if (hasItem(INVENTORY_ITEM_A_GOODLAND_TAXI_SERVICE_KEY))
                        {
                            if (hasItem(INVENTORY_ITEM_A_GTS_FIRMWARE_DRIVE))
                            {
                                game.qgts2 = true;
                                game.message = "You unlock the taxi, and insert the firmware update drive.\r\nAfter a few minutes the taxi reboots and is ready for service.";
                                setCallback(&Adventure::displayMessage);
                            }
                            else
                            {
                                game.message = "You don't have the firmware drive to repair the taxi.\r\nPerhaps you should find the company supporting these robot taxis.";
                                setCallback(&Adventure::displayMessage);
                            }
                        }
                        else
                        {
                            game.message = "You don't have a key to access the interior of the taxi.\r\nThere may be an auto repair shop somewhere in town.";
                            setCallback(&Adventure::displayMessage);
                        }
                    }
                }
                break;
            case 325:
                if (action == "talk")
                {
                    player.npc = 11;
                    setCallback(&Adventure::displayDialog);
                    setPromptCallback(&Adventure::talkToNPC);
                }
                break;
            case 327:
                if (action == "taxi")
                {
                    if (hasItem(INVENTORY_ITEM_A_GOODLAND_TAXI_SERVICE_KEY))
                    {
                        if (hasItem(INVENTORY_ITEM_A_GTS_FIRMWARE_DRIVE))
                        {
                            game.qgts3 = true;
                            game.message = "You unlock the taxi, and insert the firmware update drive.\r\nAfter a few minutes the taxi reboots and is ready for service.";
                            setCallback(&Adventure::displayMessage);
                        }
                        else
                        {
                            game.message = "You don't have the firmware drive to repair the taxi.\r\nPerhaps you should find the company supporting these robot taxis.";
                            setCallback(&Adventure::displayMessage);
                        }
                    }
                    else
                    {
                        game.message = "You don't have a key to access the interior of the taxi.\r\nThere may be an auto repair shop somewhere in town.";
                        setCallback(&Adventure::displayMessage);
                    }
                }
                break;
            case 331:
                if (action == "talk")
                {
                    player.npc = 12;
                    setCallback(&Adventure::displayDialog);
                    setPromptCallback(&Adventure::talkToNPC);
                }
                break;
            case 340:
                if (action == "leg")
                {
                    addItem(INVENTORY_ITEM_A_HUGE_TURKEY_LEG);
                    printFlag("OzSecCTF{1ts_st1ll_g00d}");
                    game.message = "You pick up a huge turkey leg.";
                    setCallback(&Adventure::displayMessage);
                }
                else
                {
                    if (action == "taxi")
                    {
                        if (hasItem(INVENTORY_ITEM_A_GOODLAND_TAXI_SERVICE_KEY))
                        {
                            if (hasItem(INVENTORY_ITEM_A_GTS_FIRMWARE_DRIVE))
                            {
                                game.qgts4 = true;
                                game.message = "You unlock the taxi, and insert the firmware update drive.\r\nAfter a few minutes the taxi reboots and is ready for service.";
                                setCallback(&Adventure::displayMessage);
                            }
                            else
                            {
                                game.message = "You don't have the firmware drive to repair the taxi.\r\nPerhaps you should find the company supporting these robot taxis.";
                                setCallback(&Adventure::displayMessage);
                            }
                        }
                        else
                        {
                            game.message = "You don't have a key to access the interior of the taxi.\r\nThere may be an auto repair shop somewhere in town.";
                            setCallback(&Adventure::displayMessage);
                        }
                    }
                }
                break;
            case 349:
                if (action == "taxi")
                {
                    if (hasItem(INVENTORY_ITEM_A_GOODLAND_TAXI_SERVICE_KEY))
                    {
                        if (hasItem(INVENTORY_ITEM_A_GTS_FIRMWARE_DRIVE))
                        {
                            game.qgts5 = true;
                            game.message = "You unlock the taxi, and insert the firmware update drive.\r\nAfter a few minutes the taxi reboots and is ready for service.";
                            setCallback(&Adventure::displayMessage);
                        }
                        else
                        {
                            game.message = "You don't have the firmware drive to repair the taxi.\r\nPerhaps you should find the company supporting these robot taxis.";
                            setCallback(&Adventure::displayMessage);
                        }
                    }
                    else
                    {
                        game.message = "You don't have a key to access the interior of the taxi.\r\nThere may be an auto repair shop somewhere in town.";
                        setCallback(&Adventure::displayMessage);
                    }
                }
                break;
                // END Goodland actions

                // BEGIN Dodge City actions
            case 350:
                if (action == "flipper")
                {
                    addItem(INVENTORY_ITEM_A_FLIPPER_ZERO);
                    game.message = "You pick up the Flipper Zero, and examin the note.\r\n\r\nTraveler, I have been captured by the local authorities. Please get me out of here.\r\nTake this Flipper Zero and use it to help me escape. - The Associate";
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 353:
                if (action == "replay")
                {
                    if (hasItem(INVENTORY_ITEM_A_FLIPPER_ZERO))
                    {
                        if (game.qdcsherrif)
                        {
                            game.qdcassociate = true;
                            printFlag("OzSecCTF{Ag3nt_1337}");
                            game.message = "You replay the signal and the cell door unlocks. The Associate is free!\r\nThe Associate: I'll meet you on the train, find a way to get us to Wichita.\r\nThe Associate then bolts out the dodor.";
                            setCallback(&Adventure::displayMessage);
                        }
                        else
                        {
                            game.message = "You look through the Flipper's recorded keycards, nothing seems to work on the cell door.";
                            setCallback(&Adventure::displayMessage);
                        }
                    }
                    else
                    {
                        game.message = "You don't have a Flipper Zero to use. You think you saw one when you first entered town.";
                        setCallback(&Adventure::displayMessage);
                    }
                }
                break;

            case 355:
                if (action == "clone")
                {
                    if (hasItem(INVENTORY_ITEM_A_FLIPPER_ZERO))
                    {
                        game.qdcsherrif = true;
                        game.message = "You use the Flipper and slowly walk past the Sherrif. The Flipper beeps and you have a copy of the keycard.";
                        setCallback(&Adventure::displayMessage);
                    }
                    else
                    {
                        game.message = "You don't have a Flipper Zero to clone the keycard. You might have seen one when you first entered town.";
                        setCallback(&Adventure::displayMessage);
                    }
                }
                break;

            case 356:
                if (action == "drink")
                {
                    addItem(INVENTORY_ITEM_A_GLASS_OF_THE_SALOON_SPECIAL);
                    game.message = "The bartender hands you a glass of the Saloon Special.";
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 357:
                if (action == "play")
                {
                    if (hasItem(INVENTORY_ITEM_A_GLASS_OF_THE_SALOON_SPECIAL))
                    {
                        printFlag("OzSecCTF{S@l00n_Sp3ci@l}");
                        game.message = "The player takes the drink and downs it. He stands up and says 'Thanks! I'd offer you to take my place, but I just lost everything. Have a great day.'";
                        setCallback(&Adventure::displayMessage);
                    }
                    else if (hasItem(INVENTORY_ITEM_AN_ENERGY_DRINK))
                    {
                        printFlag("OzSecCTF{S@l00n_Sp3ci@liz3d}");
                        game.message = "The player takes the energy drink and downs it. He stands up and says 'Awesome! I feel great! I'll take my winnings and head out. Have a great day.'";
                        setCallback(&Adventure::displayMessage);
                    }
                    else
                    {
                        game.message = "The player says 'Hey, I'm in a bind, and could use a drink of something to pick me up. I'll let you take my place if you can get me something.";
                        setCallback(&Adventure::displayMessage);
                    }
                }
                break;

            case 365:
                if (action == "drink")
                {
                    addItem(INVENTORY_ITEM_AN_ENERGY_DRINK);
                    game.message = "You pick up an energy drink.";
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 369:
                if (action == "flipper")
                {
                    addItem(INVENTORY_ITEM_A_FLIPPER_ZERO);
                    game.message = "You pick up the Flipper Zero, and examin the note.\r\n\r\nTraveler, I have been captured by the local authorities. Please get me out of here.\r\nTake this Flipper Zero and use it to help me escape. - The Associate";
                    setCallback(&Adventure::displayMessage);
                }
                break;

            case 371:
                if (action == "clone")
                {
                    if (hasItem(INVENTORY_ITEM_A_FLIPPER_ZERO))
                    {
                        game.qdcconductor = true;
                        game.message = "You use the Flipper slowly approach the conductor. The Flipper beeps and you now have a copy of his keycard.";
                        setCallback(&Adventure::displayMessage);
                    }
                    else
                    {
                        game.message = "You don't have a Flipper Zero to clone the keycard. You might have seen one when you first entered town.";
                        setCallback(&Adventure::displayMessage);
                    }
                }
                break;

            case 372:
                if (action == "replay")
                {
                    if (hasItem(INVENTORY_ITEM_A_FLIPPER_ZERO))
                    {
                        if (game.qdcconductor)
                        {
                            game.qdcassociate = true;
                            printFlag("OzSecCTF{Tr@in_Unl0ck3d}");
                            game.message = "You replay the signal and the train door unlocks.";
                            setCallback(&Adventure::displayMessage);
                        }
                        else
                        {
                            game.message = "You look through the Flipper's recorded keycards, nothing seems to work on the train.";
                            setCallback(&Adventure::displayMessage);
                        }
                    }
                    else
                    {
                        game.message = "You don't have a Flipper Zero to use. You think you saw one when you first entered town.";
                        setCallback(&Adventure::displayMessage);
                    }
                }
                break;
                // END Dodge City actions
                // BEGIN Wichita actions
            case 485:
                if (action == "line")
                {
                    Serial.println("You join the line.");
                    sleep(5);
                    Serial.println("The line isn't moving.");
                    sleep(5);
                    game.message = "You decide to leave the line.";
                    setCallback(&Adventure::displayMessage);
                }
                else if (action == "reboot")
                {
                    if (hasItem(INVENTORY_ITEM_A_RECOVERY_DRIVE))
                    {
                        game.qictair1 = true;
                        game.message = "You reboot the PC with the recovery drive inserted. It boots back into Windows successfully.";
                        setCallback(&Adventure::displayMessage);
                    }
                    else
                    {
                        game.message = "You don't have the recovery drive to reboot the server.";
                        setCallback(&Adventure::displayMessage);
                    }
                }
                break;
            case 486:
                if (action == "talk")
                {
                    player.npc = 10; // Airport IT Manager
                    setCallback(&Adventure::displayDialog);
                    setPromptCallback(&Adventure::talkToNPC);
                }
                else if (action == "drive")
                {
                    addItem(INVENTORY_ITEM_A_RECOVERY_DRIVE);
                    game.message = "You pick up the USB recovery drive.";
                    setCallback(&Adventure::displayMessage);
                }
                break;
            case 488:
                if (action == "reboot")
                {
                    if (hasItem(INVENTORY_ITEM_A_RECOVERY_DRIVE))
                    {
                        game.qictair2 = true;
                        game.message = "You reboot the PC with the recovery drive inserted. It boots back into Windows successfully.";
                        setCallback(&Adventure::displayMessage);
                    }
                    else
                    {
                        game.message = "You don't have the recovery drive to reboot the server.";
                        setCallback(&Adventure::displayMessage);
                    }
                }
                break;
            case 491:
                if (action == "key")
                {
                    addItem(INVENTORY_ITEM_AIRPORT_KEYS);
                    game.message = "You pick up the keys.";
                    setCallback(&Adventure::displayMessage);
                }
                else if (action == "reboot")
                {
                    if (hasItem(INVENTORY_ITEM_A_RECOVERY_DRIVE))
                    {
                        game.qictair3 = true;
                        game.message = "You reboot the PC with the recovery drive inserted. It boots back into Windows successfully.";
                        setCallback(&Adventure::displayMessage);
                    }
                    else
                    {
                        game.message = "You don't have the recovery drive to reboot the server.";
                        setCallback(&Adventure::displayMessage);
                    }
                }
                break;
            case 492:
                if (action == "reboot")
                {
                    if (hasItem(INVENTORY_ITEM_A_RECOVERY_DRIVE))
                    {
                        game.qictair4 = true;
                        game.message = "You reboot the PC with the recovery drive inserted. It boots back into Windows successfully.";
                        setCallback(&Adventure::displayMessage);
                    }
                    else
                    {
                        game.message = "You don't have the recovery drive to reboot the server.";
                        setCallback(&Adventure::displayMessage);
                    }
                }
                break;
            case 494:
                if (action == "unlock")
                {
                    if (hasItem(INVENTORY_ITEM_AIRPORT_KEYS))
                    {
                        game.qictairunlock = true;
                        game.message = "You unlock the door with the airport keys.";
                        setCallback(&Adventure::displayMessage);
                    }
                    else
                    {
                        game.message = "You don't have the keys to unlock the door.";
                        setCallback(&Adventure::displayMessage);
                    }
                }
                break;
            case 495:
                if (action == "reboot")
                {
                    if (hasItem(INVENTORY_ITEM_A_RECOVERY_DRIVE))
                    {
                        game.qictair5 = true;
                        game.message = "You reboot the PC with the recovery drive inserted. It boots back into Windows successfully.";
                        setCallback(&Adventure::displayMessage);
                    }
                    else
                    {
                        game.message = "You don't have the recovery drive to reboot the server.";
                        setCallback(&Adventure::displayMessage);
                    }
                }
                break;
            case 496:
                if (action == "ticket")
                {
                    Serial.println("You grab a lottery ticket.");
                    addItem(INVENTORY_ITEM_A_LOTTERY_TICKET);
                }
                else if (action == "drink")
                {
                    Serial.println("You grab an energy drink.");
                    addItem(INVENTORY_ITEM_AN_ENERGY_DRINK);
                }
                break;
            case 497:
                if (action == "simon")
                {
                    // @todo Implement simon says gamme
                    Serial.println("If you found this, we want to chat with you. Come find the badge makers.");
                    printFlag("OzSecCTF{S1m0n_s4ys_1s_fun}");
                }
                break;
            case 499:
                if (action == "pamphlet")
                {
                    if (hasItem(INVENTORY_ITEM_A_LOAN_PAMPHLET))
                    {
                        printFlag("OzSecCTF{Spr3@d_teh_l0ans}");
                        game.message = "You offer the loan pamphlet to the vendor.";
                        setCallback(&Adventure::displayMessage);
                    }
                    else
                    {
                        game.message = "You don't have anything to offer.";
                        setCallback(&Adventure::displayMessage);
                    }
                }
                break;
            case 500:
                if (action == "sign")
                {
                    game.message = "OzSec 2024! Wichita's Annual Cyber Security Conference!\n\nJoin us for a day of talks, workshops, and networking.\n\n";
                    setCallback(&Adventure::displayMessage);
                }
                else if (action == "beacon")
                {
                    player.beacon = player.room;
                    game.message = "You set your beacon to this location.\r\nYou can return here at any time by typing 'beacon'";
                    setCallback(&Adventure::displayMessage);
                }
                break;
            case 501:
                if (action == "talk")
                {
                    player.npc = 4; // Water Co Receptionist
                    setCallback(&Adventure::displayDialog);
                    setPromptCallback(&Adventure::talkToNPC);
                }
                break;
            case 503:
                if (action == "talk")
                {
                    player.npc = 3; // Water Co IT Manager
                    setCallback(&Adventure::displayDialog);
                    setPromptCallback(&Adventure::talkToNPC);
                }
                break;
            case 504:
                if (action == "coffee")
                {
                    addItem(INVENTORY_ITEM_A_CUP_OF_COFFEE);
                    game.message = "You pick up a coffee mug and fill it with coffee from the pot.";
                    setCallback(&Adventure::displayMessage);
                }
                break;
            case 505:
                if (action == "load")
                {
                    if (hasItem(INVENTORY_ITEM_AN_HP_LTO8_TAPE))
                    {
                        game.ictwaterloaded = true;
                        game.message = "You load the tapes into the tape drive and press the button to begin restoration.";
                        setCallback(&Adventure::displayMessage);
                    }
                    else
                    {
                        game.message = "You don't have the tapes to load.";
                        setCallback(&Adventure::displayMessage);
                    }
                }
                break;
            case 507:
                if (action == "open")
                {
                    if (hasItem(INVENTORY_ITEM_SAFE_DEPOSIT_KEY))
                    {
                        game.message = "You use the key to open the safe deposit box and retrieve a backup tape.";
                        addItem(INVENTORY_ITEM_AN_HP_LTO8_TAPE);
                        setCallback(&Adventure::displayMessage);
                    }
                    else
                    {
                        game.message = "You don't have the key to open the safe deposit box.";
                        setCallback(&Adventure::displayMessage);
                    }
                }
                break;
            case 508:
                if (action == "talk")
                {
                    player.npc = 5; // Bank teller
                    setCallback(&Adventure::displayDialog);
                    setPromptCallback(&Adventure::talkToNPC);
                }
                break;
            case 509:
                if (action == "pamphlet")
                {
                    addItem(INVENTORY_ITEM_A_LOAN_PAMPHLET);
                    game.message = "You pick up a loan pamphlet detailing all the avialable loans and offers.";
                    setCallback(&Adventure::displayMessage);
                }
                break;
                // END Wichita actions
            }
        }
    }
}

/// @brief Display dialog with NPC
void Adventure::displayDialog()
{
    if (player.npc < 0 || player.npc >= sizeof(npc) / sizeof(npc[0]))
    {
        player.npc = -1;
    }

    if (player.npc == -1)
    {
        game.message = "You're not talking to anyone.";
        setCallback(&Adventure::displayMessage);
        return;
    }
    // Print dialog
    Serial.println();
    Serial.println(npc[player.npc].name + ": " + npc[player.npc].dialog[player.dialogIndex].text);
    Serial.println("--");
    // Print valid options
    if (npc[player.npc].dialog[player.dialogIndex].response1_id != -1)
    {
        Serial.println("1> " + npc[player.npc].dialog[player.dialogIndex].response1);
    }
    if (npc[player.npc].dialog[player.dialogIndex].response2_id != -1)
    {
        Serial.println("2> " + npc[player.npc].dialog[player.dialogIndex].response2);
    }
    Serial.println("0> Bye.");

    // Set callback to handle responses
    showPrompt = true;
    setPromptCallback(&Adventure::talkToNPC);
}

/// @brief Check if the action is valid for the current room
/// @param action
/// @return bool
bool Adventure::isRoomAction(String action)
{
    // Allow "n" when in the Arcade as it's the beginning of the Konami code
    if (player.room == 497)
    {
        if (action == konamiStrings[konamiIndex])
        {
            return true;
        }
        else
        {
            konamiIndex = 0;
        }
    }
    for (int i = 0; i < MAX_ROOM_OPTIONS; i++)
    {
        if (action == rooms[player.room].options[i])
        {
            return true;
        }
    }

    return false;
}

/// @brief Display something to the player, using the storedCallback function.
void Adventure::show()
{
    // Only display if data should be displayed
    if (storedCallback)
    {
        (this->*storedCallback)();
        unsetCallback();
    }
}

/// @brief Logic when talking to an NPC and the provided dialog response
/// @param response
void Adventure::talkToNPC(String response)
{
    bool questComplete = false;

    if (response == "0" || response == "1" || response == "2")
    {
        // 0 is exit
        if (response == "0")
        {
            Serial.println("\n" + npc[player.npc].name + ": Thanks for talking to me! Bye!");
            player.npc = -1;
            player.dialogIndex = 0;
            showPrompt = true;
            unsetPromptCallback();
            return;
        }
        else if (response == "1")
        {
            if (npc[player.npc].dialog[player.dialogIndex].response1_id == -1)
            {
                Serial.println("Invalid response. Please try again.");
            }
            else
            {
                // Check if quest
                if (npc[player.npc].dialog[player.dialogIndex].response1_id == -2)
                {
                    questComplete = checkQuest(player.npc);
                    if (questComplete)
                    {
                        player.dialogIndex = player.dialogIndex + 2;
                    }
                    else
                    {
                        player.dialogIndex = player.dialogIndex + 1;
                    }
                }
                else
                {
                    player.dialogIndex = npc[player.npc].dialog[player.dialogIndex].response1_id;
                }
            }
        }
        else if (response == "2")

        {
            if (npc[player.npc].dialog[player.dialogIndex].response2_id == -1)
            {
                Serial.println("Invalid response. Please try again.");
            }
            else
            {
                // Check if quest
                if (npc[player.npc].dialog[player.dialogIndex].response2_id == -2)
                {
                    questComplete = checkQuest(player.npc);
                    if (questComplete)
                    {
                        player.dialogIndex = player.dialogIndex + 2;
                    }
                    else
                    {
                        player.dialogIndex = player.dialogIndex + 1;
                    }
                }
                else
                {
                    player.dialogIndex = npc[player.npc].dialog[player.dialogIndex].response2_id;
                }
            }
        }
    }
    else
    {
        Serial.println("Invalid response. Please try again.");
    }
    setCallback(&Adventure::displayDialog);
    setPromptCallback(&Adventure::talkToNPC);
    showPrompt = true;
}

/// @brief Checks if the specified NPC's quest is complete
/// @param npc
/// @return bool
bool Adventure::checkQuest(int npc)
{
    switch (npc)
    {
    case 0: // Training area vault door
        if (hasItem(INVENTORY_ITEM_RED_KEYCARD))
        {
            return true;
        }
        break;
    case 1:
        // Player requested to leave the training area
        completeTraining();
        return true;
        break;
    case 2: // Chanute Reporter
        if (hasItem(INVENTORY_ITEM_CHANOOGLE_SIGN))
        {
            printFlag("OzSecCTF{1-800-CHAN00G-411}");
            game.qchanute = true;
            return true;
        }
        break;
    case 3: // Wichita Water IT Manager
        // If you have the access card, you are returning because the tapes were loaded.
        if (hasItem(INVENTORY_ITEM_WICHITA_WATER_DATACENTER_ACCESS_CARD) && game.ictwaterloaded)
        {
            // Complete this quest, is Wichita itself complete?
            if (game.qictairport)
            {
                Serial.println("\r\n\r\nCongratulations! You have completed all of the main quests in the game.\r\n\r\nHere is a flag for your accomplishments:");
                printFlag("OzSecCTF{Kan5@s_1s_s@f3_4_n0w}");
                game.qwichita = true;
            }

            printFlag("OzSecCTF{W@t3r_1s_L1f3}");
            game.qictwater = true;

            return true;
        }
        // If this is the first interaction, check for the tape
        else if (hasItem(INVENTORY_ITEM_AN_HP_LTO8_TAPE))
        {
            if (!hasItem(INVENTORY_ITEM_WICHITA_WATER_DATACENTER_ACCESS_CARD)) // Only add if it's not already there.
                addItem(INVENTORY_ITEM_WICHITA_WATER_DATACENTER_ACCESS_CARD);
            game.ictwatertape = true;
            return true;
        }
        break;
    case 5: // Wichita Bank Teller
        addItem(INVENTORY_ITEM_SAFE_DEPOSIT_KEY);
        return true;
        break;
    case 6: // Pittsburg Town Hall
        if (game.qptsshutdown && game.qptsunplug)
        {
            printFlag("OzSecCTF{P1tt$burg_ATT&CK_2an_$om3_W@re}");
            game.qpittsburg = true;
            return true;
        }
        break;
    case 7: // Pittsburg Courthouse
        if (hasItem(INVENTORY_ITEM_A_LETTER_TO_THE_PITTSBURG_COURTS))
        {
            printFlag("OzSecCTF{C0urt_0f_L@w}");
            return true;
        }
        break;
    case 8: // Pittsburg Stacey
        if (hasItem(INVENTORY_ITEM_STACEYS_CUP_OF_COFFEE))
        {
            printFlag("OzSecCTF{C0ff33_1s_L1f3}");
            return true;
        }
        break;
    case 10: // Wichita Airport IT Guy
        if (game.qictair1 && game.qictair2 && game.qictair3 && game.qictair4 && game.qictair5)
        {
            if (game.qictwater)
            {
                Serial.println("\r\n\r\nCongratulations! You have completed all of the main quests in the game.\r\n\r\nHere is a flag for your accomplishments:");
                printFlag("OzSecCTF{Kan5@s_1s_s@f3_4_n0w}");
                game.qwichita = true;
            }
            printFlag("OzSecCTF{CRWD_S0urc3d_R3B00t_0verthym3}");
            game.qictairport = true;
            return true;
        }
        break;
    case 11: // Goodland Repair
        addItem(INVENTORY_ITEM_A_GOODLAND_TAXI_SERVICE_KEY);
        return true;
        break;

    case 12: // Goodland Telecom
        if (hasItem(INVENTORY_ITEM_A_GTS_FIRMWARE_DRIVE))
        {
            if (game.qgts1 && game.qgts2 && game.qgts3 && game.qgts4 && game.qgts5)
            {
                printFlag("OzSecCTF{R0b0t_T@xi_S3rv1c3}");
                game.qgoodland = true;
                return true;
            }
            else
            {
                Serial.println("We are still showing some taxi's in the field that are stuck, go make sure you can fix all of them.");
            }
            return false;
        }
        else
        {
            // No key, so this is the first visit.
            addItem(INVENTORY_ITEM_A_GTS_FIRMWARE_DRIVE);
            Serial.println("Here's a firmware flash drive, get into the taxi and update the firmware. If you need a key, check with the auto repair shop on Highway 24.");
            return false;
        }
        break;
    case 13: // KC Bus Manager
        if (game.qkcbus1024 && game.qkcbus2018 && game.qkcbus1138)
        {
            addItem(INVENTORY_ITEM_A_TICKET_TO_TOPEKA);
            return true;
        }
    case 15: // Newton IT Specialist
        if (hasItem(INVENTORY_ITEM_NEWTON_RECOVERY_PAGE_1) && hasItem(INVENTORY_ITEM_NEWTON_RECOVERY_PAGE_2) && hasItem(INVENTORY_ITEM_NEWTON_RECOVERY_PAGE_3) && hasItem(INVENTORY_ITEM_NEWTON_RECOVERY_PAGE_4))
        {
            printFlag("OzSecCTF{N3wton_R3c0v3r3d}");
            game.qnewton = true;
            return true;
        }
    default:
        return false;
    }
    return false;
}

/// @brief Display a message to the player. The message is stored in game state variables.
void Adventure::displayMessage()
{
    Serial.println(game.message);
    game.message = "";
    showPrompt = true;
    unsetCallback();
}

/// @brief Display the current room to the player. Used when the player looks.
void Adventure::displayRoom()
{
    // Make sure player.room is valid
    // Send player to starting room if it's not.
    if (player.room < 0 || player.room >= sizeof(rooms) / sizeof(rooms[0]))
    {
        player.room = GRANDLOBBY;
        game.message = "You found yourself somewhere dark and hazy, after taking a few steps you are in the Grand Lobby.";
        setCallback(&Adventure::displayMessage);
        return;
    }

    // Special case for locked rooms
    if (player.room == THEVAULT && !game.qtrainingvault)
    {
        player.room = player.previousRoom;

        Serial.println("The door is locked. Perhaps there's a key that can unlock it.");
        showPrompt = true;
        return;
    }

    // The Pittsburg University Computer Lab is only accessible from the ceiling tiles.
    if (player.room == 199 && player.previousRoom == 192)
    {
        player.room = player.previousRoom;

        Serial.println("The door is locked.");
        showPrompt = true;
        return;
    }

    // Computer lab is always locked.
    if (player.room == 192 && player.previousRoom == 199)
    {
        Serial.println("As you exit the computer lab the door slams shut behind you and locks.");
        showPrompt = true;
        return;
    }

    // Ellsworth Water Treatment Plant
    if (player.room == 291 && !game.qelaccess)
    {
        player.room = player.previousRoom;
        Serial.println("The guard intercepts you, 'Sorry, you need to checkin first.'");
        showPrompt = true;
        return;
    }

    // Dodge City Train
    if (player.room == 370 && !game.qdcconductor)
    {
        player.room = player.previousRoom;
        Serial.println("The train door is locked.");
        showPrompt = true;
        return;
    }
    if (player.room == 370 && game.qdcconductor && game.qdcassociate)
    {
        game.qdodgecity = true;
        Serial.println("The Associate thanks you for the assistance and heads out.");
        printFlag("OzSecCTF{Th3_Ass0ci@t3_0f_D0dg3_C1ty}");
    }

    // Ensure all other quests are completed before allowing you to enter Wichita.
    if ((player.room == 429 || player.room == 470 || player.room == 404 || player.room == 451))
    {
        if (!(game.qkansascity && game.qtopeka && game.qgoodland && game.qdodgecity && game.qnewton && game.qellsworth && game.qpittsburg && game.qchanute))
        {
            player.room = player.previousRoom;
            Serial.println("As you approach there is a sign with 'Road Closed' and a law enforcement officer standing\r\nnearby. The officer tells you that this area is off limits as cities around the state\r\nare experiencing cyber related incidents and need assistance.\r\n\r\nYou are then escorted back where you came.");
            showPrompt = true;
            return;
        }
    }

    if (player.room == 495 && !game.qictairunlock)
    {
        player.room = player.previousRoom;
        Serial.println("The door is locked.");
        showPrompt = true;
        return;
    }

    // Wichita water data center locked door
    if (player.room == 505)
    {
        if (!hasItem(INVENTORY_ITEM_WICHITA_WATER_DATACENTER_ACCESS_CARD))
        {
            player.room = player.previousRoom;
            Serial.println("The door is locked. You need an access card to enter.");
            showPrompt = true;
            return;
        }
        else
        {
            Serial.println("You swipe the access card and the door unlocks as you walk in.");
        }
    }

    Serial.println();
    Serial.println(rooms[player.room].title);
    Serial.println("==================");
    printWithWrapping(rooms[player.room].description, 100); // Adjust wrap width as necessary
    Serial.println("==================");
    // Dynamically print directions
    for (int i = 0; i < MAX_ROOM_NEIGHBORS; i++)
    {
        String direction = "";
        switch (i)
        {
        case 0:
            direction = "n";
            break;
        case 1:
            direction = "e";
            break;
        case 2:
            direction = "w";
            break;
        case 3:
            direction = "s";
            break;
        }
        if (rooms[player.room].neighbors[i] != -1)
        {
            Serial.print("[" + direction + "] ");
            Serial.println(rooms[rooms[player.room].neighbors[i]].title);
        }
    }

    String tempString = String(rooms[player.room].actions);
    while (tempString.indexOf("\n") != -1)
    {
        String stringToPrint = tempString.substring(0, tempString.indexOf("\n"));
        Serial.println(stringToPrint);
        tempString = tempString.substring(tempString.indexOf("\n") + 1);
    }
    unsetCallback();
    showPrompt = true;
}

void Adventure::printWithWrapping(String text, int width)
{
    int currentLineLength = 0;
    String currentWord = "";
    bool lastCharWasCR = false; // To track if the last character was a carriage return
    bool skipNewLine = false;   // To prevent extra newlines
    char c;

    for (int i = 0; i < text.length(); i++)
    {
        c = text[i];

        // Check for newline '\n' or '\r\n' cases
        if (c == '\r')
        {
            lastCharWasCR = true;
        }
        else if (c == '\n')
        {
            // If the previous character was '\r', this is part of a '\r\n' pair
            if (lastCharWasCR)
            {
                lastCharWasCR = false;
            }

            // Print the last word if necessary
            if (currentWord.length() > 0)
            {
                // Check if adding this word exceeds the width
                if (currentLineLength + currentWord.length() > width)
                {
                    Serial.println();
                    currentLineLength = 0;
                }
                Serial.print(currentWord);
                currentLineLength = currentLineLength + currentWord.length();
                currentWord = "";
            }

            // Only add a new line if the previous character was not another newline
            if (!skipNewLine)
            {
                Serial.println();
                skipNewLine = true;
            }

            currentLineLength = 0;
        }
        else
        {
            // Normal character
            if (lastCharWasCR)
            {
                // '\r' was not followed by '\n', treat it as a line break
                if (currentWord.length() > 0)
                {
                    // Check if adding this word exceeds the width
                    if (currentLineLength + currentWord.length() > width)
                    {
                        Serial.println();
                        currentLineLength = 0;
                    }
                    Serial.print(currentWord);
                    currentLineLength = currentWord.length();
                    currentWord = "";
                }
                currentLineLength = 0;
                lastCharWasCR = false;
            }

            if (c == ' ' || i == text.length() - 1)
            {
                // Include the last character in the current word if we're at the end
                if (i == text.length() - 1 && c != ' ')
                {
                    currentWord += c;
                }

                // Check if adding this word exceeds the width
                if (currentLineLength + currentWord.length() > width)
                {
                    Serial.println();
                    currentLineLength = 0;
                }

                // Print the current word and a space
                Serial.print(currentWord + (c == ' ' ? " " : ""));
                currentLineLength += currentWord.length() + (c == ' ' ? 1 : 0);
                currentWord = "";
                skipNewLine = false;
            }
            else
            {
                currentWord += c;
            }
        }
    }

    // Print any remaining word after the loop
    if (currentWord.length() > 0)
    {
        if (currentLineLength + currentWord.length() > width)
        {
            Serial.println();
        }
        Serial.print(currentWord);
    }

    Serial.println();
}

/// @brief Handle input from the player and the prompt sent.
void Adventure::prompt()
{
    static String response = ""; // Persistent buffer for input
    char incomingChar;

    // Only print the prompt once
    if (showPrompt)
    {
        showPrompt = false;
        Serial.print("> ");
    }

    // Read if there is data available
    while (Serial.available() > 0)
    {
        incomingChar = Serial.read();
        switch (incomingChar)
        {
        case '\b': // backspace
        case 127:  // delete
        {
            if (response.length() > 0)
            {
                response.remove(response.length() - 1); // Remove last character

                // Move the cursor back, overwrite the last character with a space, and move the cursor back again
                Serial.print("\b \b");
            }
            break;
        }
        case '\r':
        case '\n':
        {
            Serial.println(); // Move to the next line

            response.trim(); // Trim any extraneous spaces

            if (response.length() == 0) // Ignore empty input (just enter key)
            {
                showPrompt = true;
            }
            else
            {
                processPromptResponse(response);
                response = ""; // Clear the buffer for the next input
                showPrompt = true;
            }
            break;
        }
        default:
        {
            // Add the character to the response buffer and echo it back to the serial monitor
            response += incomingChar;
            Serial.print(incomingChar);
            break;
        }
        }
    }
}

/// @brief Process received input
void Adventure::processPromptResponse(String promptResponse)
{
    // Check if callback is set
    if (promptCallback)
    {
        (this->*promptCallback)(promptResponse);
    }
    else if (isRoomAction(promptResponse))
    {
        roomAction(promptResponse);
    }
    else
    {
        systemCommand(promptResponse);
    }

    return;
}

/// @brief Update game state based on various events.
void Adventure::stateUpdate()
{
}

/// @brief System command to display help message.
void Adventure::cmdHelp()
{
    Serial.println("Commands:");
    Serial.println("help - Display this help message.");
    Serial.println("save - Save your game.");
    Serial.println("exit - Exit the game.");
    Serial.println("notebook - Show your notebook, which includes any CTF Flags you've found.");
    Serial.println("write - Write in your notebook."); // @todo Potentially remove this? Keep it a background function?
    Serial.println("inventory - Display your inventory.");
    Serial.println("look - Display the room description again.");
    Serial.println("nickname - Change your name.");
    Serial.println("whoami - Display your name.");
    Serial.println("badge - Check your badge status.");
    Serial.println("scan - Set your badge into scanning mode.");
    Serial.println("n, s, e, w - Go in a direction.");
    Serial.println("beacon - Call a BEACON taxi service and return to your specified beacon location.");
    Serial.println("keyword - Perform action on keyword from room description.");
    Serial.println("wifi - Configure WiFi settings.");
    Serial.println("reset - Reset game state.");
    Serial.println("debug - Show game state.");
    Serial.println("twinkle - Toggle LED mode.");
    Serial.println("toggle <led> - Toggle LED on or off in adventure led mode.");
    Serial.println("LED's: 0, 1, 2, 3, 4, 5, 6, 7");
    showPrompt = true;
    unsetCallback();
}

void Adventure::cmdTwinkle()
{
    if ((lightMode == TWINKLE) && (ledTwinkleMode == 1))
    {
        lightMode = TWINKLE;
        ledTwinkleMode = 2;
        Serial.println("LED mode set to TWINKLE 2.");
    }
    else if ((lightMode == TWINKLE) && (ledTwinkleMode == 2))
    {
        lightMode = TWINKLE;
        ledTwinkleMode = 3;
        Serial.println("LED mode set to TWINKLE 3.");
    }
    else if ((lightMode == TWINKLE) && (ledTwinkleMode == 3))
    {
        lightMode = ADVENTURE;
        ledTwinkleMode = 1;
        Serial.println("LED mode set to ADVENTURE.");
    }
    else
    {
        lightMode = TWINKLE;
        ledTwinkleMode = 1;
        Serial.println("LED mode set to TWINKLE 1.");
    }
    showPrompt = true;
    unsetCallback();
}

/// @brief System command to save the game.
void Adventure::cmdSave()
{
    save();
    Serial.println("Game saved.");
    showPrompt = true;
    unsetCallback();
}

void Adventure::cmdLoad()
{
    load();
    Serial.println("Game state loaded.");
    showPrompt = true;
    unsetCallback();
}

/// @brief System command to scan for 2023 ble signals
void Adventure::cmdScan()
{
    static bool bleInit = false;

    if (!bleInit)
    {
        OzSecBLE::init();
        bleInit = true;
    }

    bool found = OzSecBLE::scan();

    if (found)
    {
        Serial.println("The badge you are carrying chirps and a green light has illuminated.");
        game.qmodel2023 = true;
        save();
        digitalWrite(GPIO_NUM_17, HIGH);
    }
    else
    {
        Serial.println("The badge you are holding beeps and a red light illuminates.");
        game.qmodel2023 = false;
        save();
        digitalWrite(GPIO_NUM_17, LOW);
    }

    showPrompt = true;
    unsetCallback();
}

/// @brief System command to exit the game and return to normal badge operation.
void Adventure::cmdExit()
{
    while (Serial.available() > 0)
    {
        Serial.read();
    }
    serialConnected = false;
    lightMode = TWINKLE;
    Serial.println("Thanks for playing!");
    setCallback(&Adventure::displayRoom);
}

/// @brief System command to configure WiFi settings.
void Adventure::cmdWifi()
{
    game.message = "Your current SSID: '" + wifiSsid + "'\r\nWould you like to change it? (y/n)";
    setCallback(&Adventure::displayMessage);
    setPromptCallback(&Adventure::confirmWifi);
}

/// @brief System command to display the player's inventory.
void Adventure::cmdInventory()
{
    for (int i = 0; i < INVENTORY_ITEM_INDEX_COUNT; i++)
    {
        if (player.inventory[i] > 0)
        {
            Serial.println(String(InventoryItems[i]) + " x " + String(player.inventory[i]));
        }
    }
    showPrompt = true;
}

/// @brief System command to display the room description again.
void Adventure::cmdLook()
{
    setCallback(&Adventure::displayRoom);
}

/// @brief System command to change the player's nickname.
void Adventure::cmdNickname()
{
    game.message = "Your current nickname is " + player.name + ".\r\nEnter your new nickname:";
    setCallback(&Adventure::displayMessage);
    setPromptCallback(&Adventure::setNickname);
}

/// @brief System command to display the player's name.
void Adventure::cmdWhoami()
{
    game.message = "You are " + player.name + ".";
    setCallback(&Adventure::displayMessage);
}

/// @brief System command to reset the game state to defaults.
void Adventure::cmdReset()
{
    preferences.begin("game-data-inventory", false);
    preferences.clear();
    preferences.end();
    preferences.begin("game-data", false);
    // Clear the preferences to reset the game state
    if (preferences.clear())
    {
        digitalWrite(GPIO_NUM_17, LOW); // Turn off the 2023 led
        load();
        game.message = "Game state has been reset.";
    }
    else
    {
        game.message = "Game state failed to reset.";
    }
    preferences.end();
    setCallback(&Adventure::displayMessage);
}

// @todo Remove this for production
/// @brief System command to display debug information.
void Adventure::cmdDebug()
{
    preferences.begin("game-data-inventory", true);
    game.message = "Inventory:\n";
    for (int i = 0; i < INVENTORY_ITEM_INDEX_COUNT; i++)
    {
        game.message += "  " + String(InventoryItems[i]) + " x " + String(player.inventory[i]) + "\n";
    }
    preferences.end();
    preferences.begin("game-data", true);
    game.message += "Current Room: " + String(player.room) + "\n";
    game.message += "Game state:\nName: " + player.name + "\n";
    game.message += "Room: " + String(player.room) + "\n";
    game.message += "Previous Room: " + String(player.previousRoom) + "\n";
    game.message += "Beacon: " + String(player.beacon) + "\n";
    // Add quest status
    game.message += "\nQuests:\nRoom 0 unlocked: " + String(game.qtrainingvault) + "\nTraining complete: " + String(game.qtraining) + "\nChanute: " + String(game.qchanute) + "\nGoodland: " + String(game.qgoodland) + "\nTaxis: " + String(game.qgts1) + String(game.qgts2) + String(game.qgts3) + String(game.qgts4) + String(game.qgts5) + "\nDodge City: " + String(game.qdodgecity) + "\nNewton: " + String(game.qnewton) + "\nEllsworth: " + String(game.qellsworth) + "\nPittsburg: " + String(game.qpittsburg) + "\nWichita: " + String(game.qwichita);
    preferences.end();

    // Display the ID's and titles of rooms that have actions
    // This was used to see what rooms needed actions added/tested.
    game.message += "\n\nRooms with actions:\n";
    for (int i = 0; i < sizeof(rooms) / sizeof(rooms[0]); i++)
    {
        if (rooms[i].actions != "")
        {
            game.message += String(i) + ": " + rooms[i].title + "\n";
        }
    }

    setCallback(&Adventure::displayMessage);
}

void Adventure::cmdGoto(int room)
{
    player.room = room;
    setCallback(&Adventure::displayRoom);
}

void Adventure::cmdToggle(int led)
{
    if (led < 0 || led >= SIMPLE_NUM_LEDS)
    {
        game.message = "Invalid LED number.";
        setCallback(&Adventure::displayMessage);
        return;
    }

    int ledPin = all_leds[led];

    if (Lights::getLedStatus(ledPin))
    {
        Lights::ledOff(ledPin, true);
        game.message = "LED " + String(led) + " on pin " + String(ledPin) + " turned off.";
    }
    else
    {
        Lights::ledOn(ledPin, true);
        game.message = "LED " + String(led) + " on pin " + String(ledPin) + " turned on.";
    }
    game.message += "\r\n";
    String ledStatusString;
    for (int i = 0; i < SIMPLE_NUM_LEDS; i++)
    {
        if (Lights::getLedStatus(all_leds[i]))
        {
            ledStatusString = "ON";
        }
        else
        {
            ledStatusString = "OFF";
        }
        game.message += "LED " + String(i) + ": " + ledStatusString + "\r\n";
    }

    setCallback(&Adventure::displayMessage);
}

/// @brief System command to check the in game badge status.
void Adventure::cmdBadge()
{
    String badgeStatus;
    if (game.qmodel2023)
    {
        badgeStatus = "green";
    }
    else
    {
        badgeStatus = "red";
    }

    game.message = "You look down at your badge and see a " + badgeStatus + " light.";
    setCallback(&Adventure::displayMessage);
}

void Adventure::cmdNotebook()
{
    game.message = player.notebook;
    setCallback(&Adventure::displayMessage);
}

void Adventure::cmdWriteNote(String note)
{
    player.notebook += note + "\r\n";
    game.message = "You write '" + note + "' in your notebook.";
    setCallback(&Adventure::displayMessage);
}

void Adventure::cmdBeacon()
{
    player.room = player.beacon;
    Serial.println("You call a BEACON taxi service and are dropped off.");
    setCallback(&Adventure::displayRoom);
}

void Adventure::cmdCompleteQuest(int quest)
{
    switch (quest)
    {
    case 0:
        game.qtrainingvault = true;
        break;
    case 1:
        game.qtraining = true;
        break;
    case 2:
        game.qchanute = true;
        break;
    case 3:
        game.qkansascity = true;
        break;
    case 4:
        game.qtopeka = true;
        break;
    case 5:
        game.qgoodland = true;
        break;
    case 6:
        game.qdodgecity = true;
        break;
    case 7:
        game.qnewton = true;
        break;
    case 8:
        game.qellsworth = true;
        break;
    case 9:
        game.qpittsburg = true;
        break;
    case 10:
        game.qwichita = true;
        break;
    default:
        break;
    }
}

void Adventure::cmdCheat(String code)
{
    if (code == "motherlode")
    {
        game.cheats = true;
        game.message = "Cheats mode enabled.";
    }
    else
    {
        game.cheats = false;
        game.message = "Invalid cheat code.";
    }
    setCallback(&Adventure::displayMessage);
}

/// @brief Handle system commands.
void Adventure::systemCommand(String command)
{

    String program = "";
    String arguments = "";

    // Split command on spaces
    if (command.indexOf(' ') == -1)
    {
        program = command;
    }
    else
    {
        int spaceIndex = command.indexOf(' ');
        if (spaceIndex != -1)
        {
            program = command.substring(0, spaceIndex);
            arguments = command.substring(spaceIndex + 1);
        }
    }

    // If tree because switch statements can't switch on strings.
    if (program == "help")
    {
        cmdHelp();
    }
    else if (program == "save")
    {
        cmdSave();
    }
    else if (program == "load")
    {
        cmdLoad();
    }
    else if (program == "exit")
    {
        cmdExit();
    }
    else if (program == "wifi")
    {
        cmdWifi();
    }
    else if (program == "inventory" || program == "i")
    {
        cmdInventory();
    }
    else if (program == "look" || program == "l")
    {
        cmdLook();
    }
    else if (program == "nickname")
    {
        cmdNickname();
    }
    else if (program == "whoami")
    {
        cmdWhoami();
    }
    else if (program == "reset")
    {
        cmdReset();
    }
    else if (program == "twinkle")
    {
        cmdTwinkle();
    }
    else if (program == "badge")
    {
        cmdBadge();
    }
    else if (program == "scan")
    {
        cmdScan();
    }
    else if (program == "notebook")
    {
        cmdNotebook();
    }
    else if (program == "write")
    {
        cmdWriteNote(arguments);
    }
    else if (program == "beacon")
    {
        cmdBeacon();
    }
    else if (program == "cheat")
    {
        cmdCheat(arguments);
    }
    else if (program == "debug")
    {
        if (game.cheats)
        {
            cmdDebug();
        }
        else
        {
            game.message = "Cheat code required to use this command.";
            setCallback(&Adventure::displayMessage);
        }
    }
    else if (program == "goto") // @todo Remove for production
    {
        if (game.cheats)
        {
            cmdGoto(arguments.toInt());
        }
        else
        {
            game.message = "Cheat code required to use this command.";
            setCallback(&Adventure::displayMessage);
        }
    }
    else if (program == "toggle")
    {
        if (game.cheats)
        {
            cmdToggle(arguments.toInt());
        }
        else
        {
            game.message = "Cheat code required to use this command.";
            setCallback(&Adventure::displayMessage);
        }
    }
    else if (program == "complete")
    {
        if (game.cheats)
        {
            cmdCompleteQuest(arguments.toInt());
        }
        else
        {
            game.message = "Cheat code required to use this command.";
            setCallback(&Adventure::displayMessage);
        }
    }
    else
    {
        game.message = "Unknown command.";
        setCallback(&Adventure::displayMessage);
    }
}

void Adventure::printFlag(String flag)
{
    Serial.println("Flag: " + flag);
    cmdWriteNote("Flag: " + flag);
    sleep(3);
}