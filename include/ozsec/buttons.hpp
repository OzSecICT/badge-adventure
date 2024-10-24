#ifndef Buttons_hpp
#define Buttons_hpp
#include <Arduino.h>
#include <OneButton.h>

// Define the buttons
#ifdef BOARD_SIMON
OneButton boot_button(GPIO_NUM_0);
OneButton select_button(GPIO_NUM_36);
OneButton up_button(GPIO_NUM_37);
OneButton down_button(GPIO_NUM_38);
OneButton left_button(GPIO_NUM_46);
OneButton right_button(GPIO_NUM_35);
#else
OneButton boot_button(GPIO_NUM_0);
OneButton select_button(GPIO_NUM_47);
OneButton up_button(GPIO_NUM_21);
OneButton down_button(GPIO_NUM_48);
OneButton left_button(GPIO_NUM_14);
OneButton right_button(GPIO_NUM_45);
#endif

#endif