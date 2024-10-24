#include <Arduino.h>
#include <Preferences.h>
#include <ozsec/rooms.hpp>
#include <ozsec/npcs.hpp>

// Preferences maintain persistent storage for badge and game state
extern Preferences preferences;

class Adventure;

// Callbacks used to handle user input and allow non-blocking serial
// access to play the game without blocking the main loop.
typedef void (Adventure::*Callback)();
typedef void (Adventure::*PromptCallback)(String);
typedef void (Adventure::*TalkCallback)(int);

enum InventoryItemIndexes
{
    INVENTORY_ITEM_A_CUP_OF_COFFEE,
    INVENTORY_ITEM_A_FLIPPER_ZERO,
    INVENTORY_ITEM_A_GLASS_OF_THE_SALOON_SPECIAL,
    INVENTORY_ITEM_A_GOODLAND_TAXI_SERVICE_KEY,
    INVENTORY_ITEM_A_GTS_FIRMWARE_DRIVE,
    INVENTORY_ITEM_A_HUGE_TURKEY_LEG,
    INVENTORY_ITEM_A_LADDER,
    INVENTORY_ITEM_A_LETTER_TO_THE_PITTSBURG_COURTS,
    INVENTORY_ITEM_A_LOAN_PAMPHLET,
    INVENTORY_ITEM_A_LOTTERY_TICKET,
    INVENTORY_ITEM_A_PLANE_TICKET_FROM_GOODLAND_MUNICPAL_AIRPORT,
    INVENTORY_ITEM_A_RASPBERRY_PI,
    INVENTORY_ITEM_A_RECOVERY_DRIVE,
    INVENTORY_ITEM_A_RED_ROSE,
    INVENTORY_ITEM_A_SODA,
    INVENTORY_ITEM_A_TICKET_TO_TOPEKA,
    INVENTORY_ITEM_A_USB_DRIVE_FROM_THE_PITTSBURG_PARK,
    INVENTORY_ITEM_AIRPORT_KEYS,
    INVENTORY_ITEM_AN_ENERGY_DRINK,
    INVENTORY_ITEM_AN_HP_LTO8_TAPE,
    INVENTORY_ITEM_BOX_OF_DONUTS,
    INVENTORY_ITEM_CHANOOGLE_SIGN,
    INVENTORY_ITEM_CHANUTE_TRIBUNE_LEAFLET,
    INVENTORY_ITEM_COFFEE_CONNECTION_BOGO_COUPON,
    INVENTORY_ITEM_ELLSWORTH_WATER_TREATMENT_KEYCARD,
    INVENTORY_ITEM_LANTERN,
    INVENTORY_ITEM_MAP,
    INVENTORY_ITEM_RED_KEYCARD,
    INVENTORY_ITEM_SAFE_DEPOSIT_KEY,
    INVENTORY_ITEM_STACEYS_CUP_OF_COFFEE,
    INVENTORY_ITEM_TWO_CUP_O_JOES,
    INVENTORY_ITEM_WICHITA_WATER_DATACENTER_ACCESS_CARD,
    INVENTORY_ITEM_NEWTON_RECOVERY_PAGE_1,
    INVENTORY_ITEM_NEWTON_RECOVERY_PAGE_2,
    INVENTORY_ITEM_NEWTON_RECOVERY_PAGE_3,
    INVENTORY_ITEM_NEWTON_RECOVERY_PAGE_4,
    INVENTORY_ITEM_NEWTON_LUNCH,
    INVENTORY_ITEM_PARKING_LOT_DRIVE,
};
#define INVENTORY_ITEM_INDEX_COUNT INVENTORY_ITEM_PARKING_LOT_DRIVE + 1

static const char *InventoryItems[]{
    "a cup of coffee",
    "a Flipper Zero",
    "a glass of the Saloon Special",
    "a Goodland Taxi Service key",
    "a GTS firmware drive",
    "a huge turkey leg",
    "a ladder",
    "a letter to the Pittsburg Courts",
    "a loan pamphlet",
    "a lottery ticket",
    "a plane ticket from Goodland Municpal Airport",
    "a Raspberry Pi",
    "a recovery drive",
    "a red rose",
    "a soda",
    "a ticket to Topeka",
    "a USB drive from the Pittsburg Park",
    "airport keys",
    "an energy drink",
    "an HP LTO-8 tape",
    "Box of donuts",
    "Chanoogle Sign",
    "Chanute Tribune Leaflet",
    "Coffee Connection BOGO Coupon",
    "Ellsworth Water Treatment Keycard",
    "Lantern",
    "Map",
    "Red Keycard",
    "Safe Deposit Key",
    "Stacey's cup of coffee",
    "Two Cup O' Joes",
    "Wichita Water Datacenter Access Card",
    "Recovery Instructions Page 1",
    "Recovery Instructions Page 2",
    "Recovery Instructions Page 3",
    "Recovery Instructions Page 4",
    "a sack lunch",
    "a USB drive from the parking lot"};

// Character specific variables
struct CharacterState
{
    int room;
    int previousRoom;
    int npc;         // The NPC we're talking to
    int dialogIndex; // The dialog entry
    String name;     // Characters name
    int inventory[INVENTORY_ITEM_INDEX_COUNT];
    String notebook;
    int beacon; // The room ID that the player wants to return to when using a beacon.
};

extern CharacterState player;

// Game wide variables and game state
struct GameState
{
    bool cheats;
    bool playing;
    bool qmodel2023;
    bool qtraining;
    bool qtrainingvault;
    bool qchanute;
    bool qpittsburg;
    bool qptsshutdown;
    bool qptsunplug;
    bool qkansascity;
    bool qkcbus1024;
    bool qkcbus1138;
    bool qkcbus2018;
    bool qtopeka;
    bool qtpkdrive1;
    bool qtpkdrive2;
    bool qtpkdrive3;
    bool qtpkdrive4;
    bool qtpkdrive5;
    bool qgoodland;
    bool qgts1;
    bool qgts2;
    bool qgts3;
    bool qgts4;
    bool qgts5;
    bool qdodgecity;
    bool qdcsherrif;
    bool qdcassociate;
    bool qdcconductor;
    bool qnewton;
    bool qnwtball;
    bool qnwtdisc;
    bool qellsworth;
    bool qellaptop;
    bool qellevels;
    bool qelaccess;
    bool qwichita;
    bool ictwatertape;
    bool ictwaterloaded;
    bool qictwater;
    bool qictair1;
    bool qictair2;
    bool qictair3;
    bool qictair4;
    bool qictair5;
    bool qictairunlock;
    bool qictairport;
    String message;
};

extern GameState game;

extern String wifiSsid;
extern String wifiPassword;

enum LightMode
{
    TWINKLE,
    ADVENTURE
};

class Adventure
{
private:
    CharacterState player;
    const Room *room;
    bool hasItem(int item);
    void addItem(int item);
    void removeItem(int item);
    void show();
    void displayMessage();
    void displayRoom();
    void printWithWrapping(String text, int width);
    void prompt();
    void systemCommand(String command);
    void setNickname(String response);
    void confirmWifi(String response);
    void setWifiSsid(String response);
    void setWifiPassword(String resonse);
    void roomAction(String action);
    bool isRoomAction(String action);
    void setCallback(Callback callback);
    void unsetCallback();
    void setPromptCallback(PromptCallback callback);
    void unsetPromptCallback();
    void setTalkCallback(TalkCallback callback);
    void unsetTalkCallback();
    void save();
    void load();
    void printHelp();
    void stateUpdate();
    void talkToNPC(String response);
    bool checkQuest(int npc);
    void displayDialog();
    void ledMap();
    void printFlag(String flag);

    // System commands
    void cmdHelp();
    void cmdSave();
    void cmdLoad();
    void cmdExit();
    void cmdWifi();
    void cmdInventory();
    void cmdLook();
    void cmdNickname();
    void cmdWhoami();
    void cmdReset();
    void cmdBadge();
    void cmdScan();
    void cmdTwinkle();
    void cmdNotebook();
    void cmdWriteNote(String note);
    void cmdBeacon();
    void cmdCheat(String code);
    void cmdDebug();
    void cmdGoto(int room);
    void cmdCompleteQuest(int quest);
    void cmdToggle(int led);

    // Debug
    void completeTraining();

public:
    GameState game;
    void init();
    void loop();
    void bgloop();
    void center_button_click();
    void processPromptResponse(String promptResponse);
};