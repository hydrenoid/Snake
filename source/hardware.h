//
// Created by Jonathon Moore on 11/1/2022.
//
#include "structures.h"
#ifndef UNTITLED2_HARDWARE_H


// extern because it is used in another module
extern void initializeHardware();
extern void lcdDisplayText(char text[]);
extern void ledDisplay(int num);
extern void lcdDisplayUpdate(Snake *snake, Position food);
extern int checkButton1();
extern int checkButton2();
extern void cleanupHardware();

#define UNTITLED2_HARDWARE_H

#endif //UNTITLED2_HARDWARE_H
