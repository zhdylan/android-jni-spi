#ifndef CONTROL_H
#define CONTROL_H

#include <spi-interface.h>

char * inttohex(int aa);
char * GetVersion(uint8_t devices);
void enterPairMode(uint8_t whichController);

int isEnterPairMode();
void setControllervibrateTest();
void setControllervibrate(uint8_t whichController,uint8_t strength,int repeatTimes ,int vibrateDuration,int silenceDuration);
void stopControllervibrate();
int getRadioStatus();
char * GetControllerSN(uint8_t whichController);
void setControllerShutdown(uint8_t whichController);
void setControllerUnbinding(uint8_t whichController);

#endif

