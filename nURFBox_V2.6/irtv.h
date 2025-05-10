#ifndef IRTV_H
#define IRTV_H

#include <U8g2lib.h>

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern unsigned long lastButtonPress;

void irtvSetup();
void irtvLoop();
void irtvCleanup();
void drawMenu();
void handleButton(int button);
void sendIRCodes(int region);
void sendSingleIRCode(uint64_t code, uint8_t protocol);
void activateIRLed();
void updateDisplay(const char* region, const char* brand, int sendIndex, int totalIndex);
void drawBrandSelectionMenu(int region);
void handleBrandSelection(int button, int region);
void drawOptionsMenu();
void handleOptionsMenu(int button);
void saveSettings();
void loadSettings();
void sendTargetedBrand(int region, int brandIndex);

#endif