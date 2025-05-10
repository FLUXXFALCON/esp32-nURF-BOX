#include <Arduino.h>
#include <SPI.h>
#include "analyzer.h"
#include "setting.h"

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern Adafruit_NeoPixel pixels;

// NRF24L01 Registers
#define NRF24_CONFIG      0x00
#define NRF24_EN_AA       0x01
#define NRF24_RF_CH       0x05
#define NRF24_RF_SETUP    0x06
#define NRF24_RPD         0x09

// Display constants
#define SCREEN_WIDTH      128
#define SCREEN_HEIGHT     64

// Analyzer constants
#define N                 128
#define CHANNELS          64
#define CE                5
#define CSN               17
#define MAX_SCANS         50
#define SCAN_DELAY_US     128

// Global variables
uint8_t values[N];
int CHannel[CHANNELS];

// NRF24L01 low-level functions
byte getregister(byte r) {
    digitalWrite(CSN, LOW);
    SPI.transfer(r & 0x1F);
    byte result = SPI.transfer(0);
    digitalWrite(CSN, HIGH);
    return result;
}

void setregister(byte r, byte v) {
    digitalWrite(CSN, LOW);
    SPI.transfer((r & 0x1F) | 0x20);
    SPI.transfer(v);
    digitalWrite(CSN, HIGH);
}

void powerup(void) {
    setregister(NRF24_CONFIG, getregister(NRF24_CONFIG) | 0x02);
    delay(5); // Increased delay for stability
}

void powerdown(void) {
    setregister(NRF24_CONFIG, getregister(NRF24_CONFIG) & ~0x02);
}

void ENable(void) {
    digitalWrite(CE, HIGH);
    delayMicroseconds(130); // Added settling time
}

void DIsable(void) {
    digitalWrite(CE, LOW);
}

void setrx(void) {
    setregister(NRF24_CONFIG, getregister(NRF24_CONFIG) | 0x01);
    ENable();
    delayMicroseconds(100);
}

void ScanChannels(void) {
    DIsable();
    memset(CHannel, 0, sizeof(CHannel));
    
    for (uint8_t i = 0; i < CHANNELS; i++) {
        setNeoPixelColour("purple");
        uint8_t channel = (128 * i) / CHANNELS;
        if (channel > 126) channel = 126; // Ensure valid channel
        setregister(NRF24_RF_CH, channel);
        setrx();
        delayMicroseconds(SCAN_DELAY_US);
        if (getregister(NRF24_RPD) > 0) {
            CHannel[i]++;
        }
        DIsable();
    }
    setNeoPixelColour("0");
}

void writeRegister(uint8_t reg, uint8_t value) {
    digitalWrite(CSN, LOW);
    SPI.transfer(reg | 0x20);
    SPI.transfer(value);
    digitalWrite(CSN, HIGH);
}

uint8_t readRegister(uint8_t reg) {
    digitalWrite(CSN, LOW);
    SPI.transfer(reg & 0x1F);
    uint8_t result = SPI.transfer(0x00);
    digitalWrite(CSN, HIGH);
    return result;
}

void setChannel(uint8_t CHannel) {
    if (CHannel > 126) CHannel = 126; // Ensure valid channel
    writeRegister(NRF24_RF_CH, CHannel);
}

void powerUP() {
    uint8_t config = readRegister(NRF24_CONFIG);
    writeRegister(NRF24_CONFIG, config | 0x02);
    delay(5); // Wait for power-up
}

void powerDOWN() {
    uint8_t config = readRegister(NRF24_CONFIG);
    writeRegister(NRF24_CONFIG, config & ~0x02);
}

void startListening() {
    digitalWrite(CE, HIGH);
    delayMicroseconds(130);
}

void stopListening() {
    digitalWrite(CE, LOW);
}

bool carrierDetected() {
    return readRegister(NRF24_RPD) & 0x01;
}

void analyzerSetup() {
    Serial.begin(115200);
    
    // Disable Bluetooth and WiFi to reduce interference
    esp_bt_controller_deinit();
    esp_wifi_stop();
    esp_wifi_deinit();
    
    // Initialize pins
    pinMode(CE, OUTPUT);
    pinMode(CSN, OUTPUT);
    digitalWrite(CSN, HIGH);
    digitalWrite(CE, LOW);

    // Initialize SPI
    SPI.begin(18, 19, 23, 17);
    SPI.setDataMode(SPI_MODE0);
    SPI.setFrequency(10000000);
    SPI.setBitOrder(MSBFIRST);

    // Initialize NRF24L01
    DIsable();
    powerUP();
    writeRegister(NRF24_EN_AA, 0x00);      // Disable auto-acknowledgment
    writeRegister(NRF24_RF_SETUP, 0x0F);   // Set RF parameters

    // Initialize display
    if (!u8g2.begin()) {
        Serial.println("Display init failed!");
    }
    
    // Initialize NeoPixel
    pixels.begin();
    setNeoPixelColour("0");
}

void analyzerLoop() {
    ScanChannels();
    memset(values, 0, sizeof(values));

    // Perform multiple scans
    for (uint8_t n = 0; n < MAX_SCANS; n++) {
        for (uint8_t i = 0; i < N; i++) {
            setChannel(i);
            startListening();
            delayMicroseconds(SCAN_DELAY_US);
            if (carrierDetected()) {
                values[i]++;
            }
            stopListening();
        }
    }

    // Update display
    u8g2.clearBuffer();
    int barWidth = SCREEN_WIDTH / N;
    int maxHeight = SCREEN_HEIGHT - 10; // Reserve space for text
    
    for (uint8_t i = 0; i < N; i++) {
        // Scale values to display height
        int height = (values[i] * maxHeight) / MAX_SCANS;
        if (height > maxHeight) height = maxHeight;
        u8g2.drawVLine(i * barWidth, maxHeight - height, height);
    }

    // Improved channel markers
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.setCursor(0, SCREEN_HEIGHT);
    u8g2.print("1    25    50    75    100   125");
    u8g2.sendBuffer();
}