#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <U8g2lib.h>
#include "irtv.h"

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

#define IR_LED_PIN 13
IRsend irsend(IR_LED_PIN);

#define MENU_MAIN 0
#define MENU_EU 1
#define MENU_TR 2
#define MENU_OPTIONS 3
int currentMenu = MENU_MAIN;
int selectedOption = 0;
int maxOptions = 4;

#define BTN_UP 26
#define BTN_DOWN 33
#define BTN_SELECT 27
#define BTN_BACK 25
#define BTN_EXIT 32

unsigned long lastButtonPress = 0;
const int DEBOUNCE_DELAY = 100;
const int IR_SEND_COUNT = 3;
const int IR_SEND_DELAY = 50;
const int BRAND_DELAY = 200;

bool irLedActive = false;
unsigned long irLedStartTime = 0;
const int IR_LED_INDICATOR_DURATION = 300;

int sendRepeatCount = 3;
int sendBetweenDelay = 50;
int sendBrandDelay = 200;
bool quickScanMode = true;

const char* euBrands[] = {
  "Samsung", "LG", "Sony", "Philips", "Panasonic",
  "Grundig", "Toshiba", "Sharp", "JVC", "Hisense",
  "Blaupunkt", "Hitachi", "Loewe", "Sanyo", "Telefunken",
  "Thomson", "Pioneer", "Bang & Olufsen", "Finlux", "Nordmende",
  "Medion", "Metz", "Orion", "Salora", "TCL",
  "Haier", "Huawei", "Xiaomi", "Nokia", "Saba"
};
const uint64_t euPowerCodes[] = {
  0x20DF10EF, 0x20DF10EF, 0x20DF10EF, 0x20DF10EF, 0x20DF10EF,
  0x20DF10EF, 0x20DF10EF, 0x20DF10EF, 0x20DF10EF, 0x20DF10EF,
  0x20DF10EF, 0x20DF10EF, 0x20DF10EF, 0x20DF10EF, 0x20DF10EF,
  0x20DF10EF, 0x20DF10EF, 0x20DF10EF, 0x20DF10EF, 0x20DF10EF,
  0x20DF10EF, 0x20DF10EF, 0x20DF10EF, 0x20DF10EF, 0x20DF10EF,
  0x20DF10EF, 0x20DF10EF, 0x20DF10EF, 0x20DF10EF, 0x20DF10EF
};
const uint8_t euProtocols[] = {
  NEC, NEC, NEC, NEC, NEC,
  NEC, NEC, NEC, NEC, NEC,
  NEC, NEC, NEC, NEC, NEC,
  NEC, NEC, NEC, NEC, NEC,
  NEC, NEC, NEC, NEC, NEC,
  NEC, NEC, NEC, NEC, NEC
};
const int euBrandCount = sizeof(euBrands) / sizeof(euBrands[0]);
const int euCodeCount = sizeof(euPowerCodes) / sizeof(euPowerCodes[0]);
const int euBrandCodeIndices[] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
  10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
  20, 21, 22, 23, 24, 25, 26, 27, 28, 29
};

const char* trBrands[] = {
  "Vestel", "Arçelik", "Beko", "Sunny", "Altus",
  "SEG", "Profilo", "Nexon", "Onvo", "Awox",
  "Regal", "TCL", "Grundig", "Finlux", "Hi-Level",
  "Telefunken", "Toshiba", "Axen", "Skytech", "Woon",
  "Dijitsu", "Arnica", "Nikon", "Luxor", "Rowell",
  "Navitech", "Nordmende", "Philips", "Samsung", "LG"
};
const uint64_t trPowerCodes[] = {
  0x20DF10EF, 0x20DF10EF, 0x20DF10EF, 0x20DF10EF, 0xA2A062,
  0x20DF10EF, 0x20DF10EF, 0x20DF10EF, 0x20DF10EF, 0x20DF10EF,
  0x20DF10EF, 0x20DF10EF, 0x20DF10EF, 0x20DF10EF, 0x20DF10EF,
  0x20DF10EF, 0x20DF10EF, 0x20DF10EF, 0x20DF10EF, 0x20DF10EF,
  0x20DF10EF, 0x20DF10EF, 0x20DF10EF, 0x20DF10EF, 0x20DF10EF,
  0x20DF10EF, 0x20DF10EF, 0x20DF10EF, 0x20DF10EF, 0x20DF10EF
};
const uint8_t trProtocols[] = {
  NEC, NEC, NEC, NEC, RC5,
  NEC, NEC, NEC, NEC, NEC,
  NEC, NEC, NEC, NEC, NEC,
  NEC, NEC, NEC, NEC, NEC,
  NEC, NEC, NEC, NEC, NEC,
  NEC, NEC, NEC, NEC, NEC
};
const int trBrandCount = sizeof(trBrands) / sizeof(trBrands[0]);
const int trCodeCount = sizeof(trPowerCodes) / sizeof(trPowerCodes[0]);
const int trBrandCodeIndices[] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
  10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
  20, 21, 22, 23, 24, 25, 26, 27, 28, 29
};

bool buttonStates[] = {HIGH, HIGH, HIGH, HIGH, HIGH};
bool lastButtonStates[] = {HIGH, HIGH, HIGH, HIGH, HIGH};
bool isProcessing = false;

void drawMenu();
void handleButton(int button);
void sendIRCodes(int region);
void sendSingleIRCode(uint64_t code, uint8_t protocol);
void activateIRLed();
void updateDisplay(const char* region, const char* brand, int sendIndex, int totalIndex);
void drawOptionsMenu();
void handleOptionsMenu(int button);
void saveSettings();
void loadSettings();
void irtvCleanup();

void activateIRLed() {
  irLedActive = true;
  irLedStartTime = millis();
  drawMenu();
}

void updateDisplay(const char* region, const char* brand, int sendIndex, int totalIndex) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(10, 10, region);
  char buffer[32];
  snprintf(buffer, sizeof(buffer), "Marka: %s", brand);
  u8g2.drawStr(10, 25, buffer);
  snprintf(buffer, sizeof(buffer), "Kod: %d/%d", sendIndex + 1, totalIndex);
  u8g2.drawStr(10, 35, buffer);
  u8g2.drawStr(10, 64, "B: Durdur, E: Cik");
  u8g2.sendBuffer();
}

void sendSingleIRCode(uint64_t code, uint8_t protocol) {
  for (int i = 0; i < sendRepeatCount; i++) {
    switch (protocol) {
      case NEC:
        irsend.sendNEC(code, 32);
        break;
      case RC5:
        irsend.sendRC5(code, 12);
        break;
      default:
        irsend.sendNEC(code, 32);
    }
    delay(sendBetweenDelay);
  }
}

void sendIRCodes(int region) {
  const char** brands;
  const uint64_t* codes;
  const uint8_t* protocols;
  const int* brandCodeIndices;
  int brandCount, codeCount;
  const char* regionName;

  if (region == MENU_EU) {
    brands = euBrands;
    codes = euPowerCodes;
    protocols = euProtocols;
    brandCodeIndices = euBrandCodeIndices;
    brandCount = euBrandCount;
    codeCount = euCodeCount;
    regionName = "Avrupa IR";
  } else {
    brands = trBrands;
    codes = trPowerCodes;
    protocols = trProtocols;
    brandCodeIndices = trBrandCodeIndices;
    brandCount = trBrandCount;
    codeCount = trCodeCount;
    regionName = "Turkiye IR";
  }

  for (int i = 0; i < brandCount; i++) {
    int codeIndex = brandCodeIndices[i];
    updateDisplay(regionName, brands[i], i, brandCount);
    sendSingleIRCode(codes[codeIndex], protocols[codeIndex]);
    activateIRLed();

    if ((digitalRead(BTN_BACK) == LOW || digitalRead(BTN_EXIT) == LOW) && (millis() - lastButtonPress > DEBOUNCE_DELAY)) {
      lastButtonPress = millis();
      currentMenu = MENU_MAIN;
      selectedOption = 0;
      maxOptions = 4;
      drawMenu();
      return;
    }
    delay(sendBrandDelay);
  }

  currentMenu = MENU_MAIN;
  selectedOption = 0;
  maxOptions = 4;
  drawMenu();
}

void handleOptionsMenu(int button) {
  lastButtonPress = millis();

  switch (button) {
    case BTN_UP:
      selectedOption = (selectedOption > 0) ? selectedOption - 1 : maxOptions - 1;
      drawOptionsMenu();
      break;

    case BTN_DOWN:
      selectedOption = (selectedOption < maxOptions - 1) ? selectedOption + 1 : 0;
      drawOptionsMenu();
      break;

    case BTN_SELECT:
      if (selectedOption == 0) {
        sendRepeatCount = (sendRepeatCount < 10) ? sendRepeatCount + 1 : 1;
      } else if (selectedOption == 1) {
        sendBetweenDelay = (sendBetweenDelay < 200) ? sendBetweenDelay + 10 : 10;
      } else if (selectedOption == 2) {
        sendBrandDelay = (sendBrandDelay < 1000) ? sendBrandDelay + 50 : 100;
      } else if (selectedOption == 3) {
        quickScanMode = !quickScanMode;
      } else if (selectedOption == 4) {
        currentMenu = MENU_MAIN;
        selectedOption = 2;
        maxOptions = 4;
        saveSettings();
        drawMenu();
      }
      drawOptionsMenu();
      break;

    case BTN_BACK:
    case BTN_EXIT:
      currentMenu = MENU_MAIN;
      selectedOption = 2;
      maxOptions = 4;
      saveSettings();
      drawMenu();
      break;
  }
}

void saveSettings() {
  Serial.println("Ayarlar kaydedildi");
  Serial.print("Tekrar sayisi: "); Serial.println(sendRepeatCount);
  Serial.print("Kod gecikmesi: "); Serial.println(sendBetweenDelay);
  Serial.print("Marka gecikmesi: "); Serial.println(sendBrandDelay);
  Serial.print("Hizli tarama: "); Serial.println(quickScanMode ? "Acik" : "Kapali");
}

void loadSettings() {
  sendRepeatCount = IR_SEND_COUNT;
  sendBetweenDelay = IR_SEND_DELAY;
  sendBrandDelay = BRAND_DELAY;
  quickScanMode = true;
}

void irtvCleanup() {
  currentMenu = MENU_MAIN;
  selectedOption = 0;
  maxOptions = 4;
  irLedActive = false;
  drawMenu();
  Serial.println("IRTV temizlendi");
}

void drawMenu() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);

  if (irLedActive) {
    u8g2.drawStr(100, 10, "IR ON");
  }

  if (currentMenu == MENU_MAIN) {
    u8g2.drawStr(10, 10, "IR TV Kontrolcusu v3");
    u8g2.drawStr(10, 25, selectedOption == 0 ? "> Avrupa (EU)" : "  Avrupa (EU)");
    u8g2.drawStr(10, 35, selectedOption == 1 ? "> Turkiye (TR)" : "  Turkiye (TR)");
    u8g2.drawStr(10, 45, selectedOption == 2 ? "> Secenekler" : "  Secenekler");
    u8g2.drawStr(10, 55, selectedOption == 3 ? "> Cik" : "  Cik");
  } else if (currentMenu == MENU_EU) {
    u8g2.drawStr(10, 10, "Avrupa IR Gonderiliyor");
    u8g2.drawStr(10, 64, "B: Durdur, E: Cik");
  } else if (currentMenu == MENU_TR) {
    u8g2.drawStr(10, 10, "Turkiye IR Gonderiliyor");
    u8g2.drawStr(10, 64, "B: Durdur, E: Cik");
  }

  u8g2.sendBuffer();
}

void drawOptionsMenu() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);

  u8g2.drawStr(10, 10, "IR Gonderim Ayarlari");

  char buffer[32];
  snprintf(buffer, sizeof(buffer), "%s Tekrar: %d", selectedOption == 0 ? ">" : " ", sendRepeatCount);
  u8g2.drawStr(10, 25, buffer);
  snprintf(buffer, sizeof(buffer), "%s Kod Gecikmesi: %d", selectedOption == 1 ? ">" : " ", sendBetweenDelay);
  u8g2.drawStr(10, 35, buffer);
  snprintf(buffer, sizeof(buffer), "%s Marka Gecikme: %d", selectedOption == 2 ? ">" : " ", sendBrandDelay);
  u8g2.drawStr(10, 45, buffer);
  snprintf(buffer, sizeof(buffer), "%s Hizli Tarama: %s", selectedOption == 3 ? ">" : " ", quickScanMode ? "Acik" : "Kapali");
  u8g2.drawStr(10, 55, buffer);
  snprintf(buffer, sizeof(buffer), "%s Geri", selectedOption == 4 ? ">" : " ");
  u8g2.drawStr(10, 64, buffer);

  u8g2.sendBuffer();
}

void irtvSetup() {
  Serial.begin(115200);
  Serial.println("IR TV Control Setup");
  irsend.begin();

  loadSettings();

  currentMenu = MENU_MAIN;
  selectedOption = 0;
  maxOptions = 4;

  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);
  pinMode(BTN_BACK, INPUT_PULLUP);
  pinMode(BTN_EXIT, INPUT_PULLUP);

  drawMenu();
}

void irtvLoop() {
  if (isProcessing) {
    return;
  }

  buttonStates[0] = digitalRead(BTN_UP);
  buttonStates[1] = digitalRead(BTN_DOWN);
  buttonStates[2] = digitalRead(BTN_SELECT);
  buttonStates[3] = digitalRead(BTN_BACK);
  buttonStates[4] = digitalRead(BTN_EXIT);

  for (int i = 0; i < 5; i++) {
    if (buttonStates[i] == LOW && lastButtonStates[i] == HIGH && (millis() - lastButtonPress > DEBOUNCE_DELAY)) {
      isProcessing = true;
      switch (i) {
        case 0: handleButton(BTN_UP); break;
        case 1: handleButton(BTN_DOWN); break;
        case 2: handleButton(BTN_SELECT); break;
        case 3: handleButton(BTN_BACK); break;
        case 4: handleButton(BTN_EXIT); break;
      }
      lastButtonPress = millis();
      break;
    }
    lastButtonStates[i] = buttonStates[i];
  }

  if (irLedActive && millis() - irLedStartTime > IR_LED_INDICATOR_DURATION) {
    irLedActive = false;
    drawMenu();
  }

  isProcessing = false;
}

void handleButton(int button) {
  if (currentMenu == MENU_OPTIONS) {
    handleOptionsMenu(button);
    return;
  }

  switch (button) {
    case BTN_UP:
      selectedOption = (selectedOption > 0) ? selectedOption - 1 : maxOptions - 1;
      drawMenu();
      break;

    case BTN_DOWN:
      selectedOption = (selectedOption < maxOptions - 1) ? selectedOption + 1 : 0;
      drawMenu();
      break;

    case BTN_SELECT:
      if (currentMenu == MENU_MAIN) {
        if (selectedOption == 0) {
          currentMenu = MENU_EU;
          drawMenu();
          sendIRCodes(MENU_EU);
        } else if (selectedOption == 1) {
          currentMenu = MENU_TR;
          drawMenu();
          sendIRCodes(MENU_TR);
        } else if (selectedOption == 2) {
          currentMenu = MENU_OPTIONS;
          selectedOption = 0;
          maxOptions = 5;
          drawOptionsMenu();
        } else if (selectedOption == 3) {
          irtvCleanup();
        }
      }
      break;

    case BTN_BACK:
      if (currentMenu == MENU_EU || currentMenu == MENU_TR || currentMenu == MENU_OPTIONS) {
        currentMenu = MENU_MAIN;
        selectedOption = 0;
        maxOptions = 4;
        drawMenu();
      }
      break;

    case BTN_EXIT:
      currentMenu = MENU_MAIN;
      selectedOption = 3;
      irtvCleanup();
      break;
  }
}