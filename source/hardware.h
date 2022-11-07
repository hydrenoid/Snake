//
// Created by Jonathon Moore on 11/1/2022.
//
#include "structures.h"
#ifndef UNTITLED2_HARDWARE_H


// extern because it is used in another module
extern void initializeUserButton();
extern void closeUserButton();
extern void initializeLCDDisplay();
extern void closeLCDScreen();
extern void ledDisplayText(char text[]);
extern void lcdDisplay(int num);
extern void ledDisplayUpdate(Snake *snake, Position food);
extern int checkButton1();
extern int checkButton2();

#define UNTITLED2_HARDWARE_H

#endif //UNTITLED2_HARDWARE_H
