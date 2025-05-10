#include <Arduino.h>
#include <RF24.h>
#include <U8g2lib.h>
#include <Adafruit_NeoPixel.h>
#include <algorithm>

extern RF24 RadioA;
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern Adafruit_NeoPixel pixels;

#define BUTTON_PIN_UP 26
#define BUTTON_PIN_DOWN 33
#define BUTTON_PIN_SELECT 25
#define BUTTON_PIN_BACK 32
#define MAX_SIGNALS 20
#define MAX_PACKET_SIZE 25
#define DEFAULT_PIPE_ADDRESS 0xE8E8F0F0E1LL
#define SCAN_DURATION 12500 // 125 channels * 100 ms = 12.5 seconds
#define COOLDOWN_DURATION 5000
#define DEBOUNCE_INTERVAL 200
#define DISPLAY_UPDATE_INTERVAL 250
#define SCAN_DISPLAY_INTERVAL 500
#define CHANNEL_HOP_INTERVAL 100
#define CHANNEL_START 1
#define CHANNEL_END 125

struct Signal {
  uint8_t data[MAX_PACKET_SIZE];
  uint8_t length;
  uint8_t channel;
  bool valid;
  unsigned long timestamp;
};

static Signal signals[MAX_SIGNALS];
static int signalCount = 0;
static int selectedIndex = 0;
static int displayStartIndex = 0;
static bool showDetails = false;
static bool isScanning = true;
static bool isBroadcasting = false;
static int currentChannel = CHANNEL_START;
static unsigned long scanStartTime = 0;
static unsigned long lastDebounceUp = 0;
static unsigned long lastDebounceDown = 0;
static unsigned long lastDebounceSelect = 0;
static unsigned long lastDebounceBack = 0;
static unsigned long lastDisplayUpdate = 0;
static unsigned long lastChannelSwitch = 0;
static bool scanComplete = false;
static bool noSignalsDetected = false;
static bool isCooldown = false;
static unsigned long cooldownStartTime = 0;

void setNeoPixelColour(const char* color) {
  if (strcmp(color, "white") == 0) {
    pixels.setPixelColor(0, pixels.Color(255, 255, 255));
  } else if (strcmp(color, "0") == 0) {
    pixels.setPixelColor(0, pixels.Color(0, 0, 0));
  } else if (strcmp(color, "red") == 0) {
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
  } else if (strcmp(color, "green") == 0) {
    pixels.setPixelColor(0, pixels.Color(0, 255, 0));
  } else if (strcmp(color, "blue") == 0) {
    pixels.setPixelColor(0, pixels.Color(0, 0, 255));
  } else if (strcmp(color, "yellow") == 0) {
    pixels.setPixelColor(0, pixels.Color(255, 255, 0));
  }
  pixels.show();
}

bool isSignalDuplicate(const Signal& newSignal) {
  for (int i = 0; i < signalCount; i++) {
    if (signals[i].length == newSignal.length && 
        signals[i].channel == newSignal.channel) {
      bool same = true;
      for (int j = 0; j < newSignal.length; j++) {
        if (signals[i].data[j] != newSignal.data[j]) {
          same = false;
          break;
        }
      }
      if (same) return true;
    }
  }
  return false;
}

void clearSignals() {
  for (int i = 0; i < MAX_SIGNALS; i++) {
    signals[i].valid = false;
  }
  signalCount = 0;
  selectedIndex = 0;
  displayStartIndex = 0;
}

void startScanning() {
  isScanning = true;
  scanComplete = false;
  noSignalsDetected = false;
  isCooldown = false;
  scanStartTime = millis();
  currentChannel = CHANNEL_START;
  RadioA.setChannel(currentChannel);
  
  clearSignals();
  
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 10, "Starting Scan");
  u8g2.sendBuffer();
  delay(500);
}

void displayScanProgress() {
  unsigned long elapsedTime = millis() - scanStartTime;
  int progress = map(elapsedTime, 0, SCAN_DURATION, 0, 100);
  progress = constrain(progress, 0, 100);
  
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(0, 10, "Scanning RF Signals");
  
  String channelInfo = "CH: " + String(currentChannel);
  u8g2.drawStr(0, 20, channelInfo.c_str());
  
  String signalInfo = "Found: " + String(signalCount);
  u8g2.drawStr(0, 30, signalInfo.c_str());
  
  u8g2.drawFrame(0, 40, 128, 10);
  u8g2.drawBox(0, 40, (progress * 128) / 100, 10);
  
  u8g2.drawStr(0, 60, "Please wait...");
  u8g2.sendBuffer();
}

void signalclonerSetup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN_UP, INPUT_PULLUP);
  pinMode(BUTTON_PIN_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_PIN_SELECT, INPUT_PULLUP);
  pinMode(BUTTON_PIN_BACK, INPUT_PULLUP);
  
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x10_tr);
  
  pixels.begin();
  pixels.setPixelColor(0, pixels.Color(0, 0, 255));
  pixels.show();
  
  RadioA.begin();
  RadioA.setAutoAck(false);
  RadioA.setPALevel(RF24_PA_MAX);
  RadioA.setDataRate(RF24_2MBPS);
  RadioA.setChannel(currentChannel);
  RadioA.openReadingPipe(1, DEFAULT_PIPE_ADDRESS);
  RadioA.startListening();
  
  clearSignals();
  
  for (int cycle = 0; cycle < 2; cycle++) {
    for (int i = 0; i < 3; i++) {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.drawStr(0, 10, "RF Signal Cloner");
      u8g2.drawStr(0, 30, "Initializing");
      
      String dots = "";
      for (int j = 0; j <= i; j++) {
        dots += ".";
      }
      
      setNeoPixelColour("white");
      delay(100);
      setNeoPixelColour("blue");
      
      u8g2.drawStr(75, 30, dots.c_str());
      u8g2.sendBuffer();
      delay(200);
    }
  }
  
  startScanning();
}

void updateSignalClonerDisplay() {
  if (millis() - lastDisplayUpdate < (isScanning ? SCAN_DISPLAY_INTERVAL : DISPLAY_UPDATE_INTERVAL)) return;
  
  if (isScanning) {
    displayScanProgress();
  }
  else if (noSignalsDetected) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(0, 10, "No signals detected!");
    if (isCooldown) {
      unsigned long elapsedCooldown = millis() - cooldownStartTime;
      int remainingSeconds = (COOLDOWN_DURATION - elapsedCooldown) / 1000 + 1;
      remainingSeconds = constrain(remainingSeconds, 0, COOLDOWN_DURATION / 1000);
      String cooldownMsg = "Rescan in: " + String(remainingSeconds) + "s";
      u8g2.drawStr(0, 20, cooldownMsg.c_str());
      u8g2.drawStr(0, 30, "Please wait...");
      u8g2.drawStr(0, 40, "UP: Rescan");
      u8g2.drawStr(0, 50, "DOWN: Menu");
    } else {
      startScanning();
    }
    u8g2.sendBuffer();
  }
  else if (!showDetails && scanComplete) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(0, 10, "RF Signals:");
    
    if (signalCount == 0) {
      u8g2.drawStr(10, 20, "");
    } else {
      int maxDisplayItems = 4;
      int visibleItems = min(maxDisplayItems, signalCount - displayStartIndex);
      
      int scrollbarHeight = 40;
      int totalSignals = signalCount;
      int scrollPos = (displayStartIndex * scrollbarHeight) / max(totalSignals, 1);
      int scrollBarSize = (maxDisplayItems * scrollbarHeight) / max(totalSignals, 1);
      scrollBarSize = max(scrollBarSize, 4);
      
      for (int i = 0; i < visibleItems; i++) {
        int signalIndex = i + displayStartIndex;
        String signalInfo = "Sig " + String(signalIndex + 1) + " | CH " + 
                           String(signals[signalIndex].channel) + 
                           " | Len " + String(signals[signalIndex].length);
        
        if (signalIndex == selectedIndex) {
          u8g2.drawStr(0, 20 + i * 10, ">");
        }
        u8g2.drawStr(10, 20 + i * 10, signalInfo.c_str());
      }
      
      if (signalCount > maxDisplayItems) {
        u8g2.drawFrame(124, 15, 4, scrollbarHeight);
        u8g2.drawBox(124, 15 + scrollPos, 4, scrollBarSize);
      }
      
      String totalSignalsInfo = String(signalCount) + " signals";
      u8g2.drawStr(80, 60, totalSignalsInfo.c_str());
    }
    
    u8g2.drawStr(0, 60, "BACK: Rescan SEL: Details");
    u8g2.sendBuffer();
  }
  
  if (showDetails) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(0, 10, "Signal Details:");
    u8g2.setFont(u8g2_font_5x8_tr);
    
    String index = "Index: " + String(selectedIndex + 1);
    String length = "Length: " + String(signals[selectedIndex].length) + " bytes";
    String channel = "Channel: " + String(signals[selectedIndex].channel);
    String data = "Data: ";
    int previewBytes = std::min(8, static_cast<int>(signals[selectedIndex].length));
    char hexStr[17];
    for (int i = 0; i < previewBytes; i++) {
      snprintf(hexStr + (i * 2), 3, "%02X", signals[selectedIndex].data[i]);
    }
    data += String(hexStr);
    
    u8g2.drawStr(0, 20, index.c_str());
    u8g2.drawStr(0, 30, length.c_str());
    u8g2.drawStr(0, 40, channel.c_str());
    u8g2.drawStr(0, 50, data.c_str());
    u8g2.drawStr(0, 60, isBroadcasting ? "Broadcasting..." : "SEL: Broadcast BACK: Exit");
    u8g2.sendBuffer();
  }
  
  lastDisplayUpdate = millis();
}

void switchChannel() {
  currentChannel++;
  if (currentChannel > CHANNEL_END) {
    currentChannel = CHANNEL_START;
  }
  
  RadioA.setChannel(currentChannel);
}

void signalclonerLoop() {
  unsigned long currentMillis = millis();
  
  if (isCooldown && (currentMillis - cooldownStartTime) >= COOLDOWN_DURATION) {
    isCooldown = false;
  }
  
  if (isScanning && (currentMillis - lastChannelSwitch) >= CHANNEL_HOP_INTERVAL) {
    switchChannel();
    lastChannelSwitch = currentMillis;
  }
  
  if (isScanning && ((currentMillis - scanStartTime) >= SCAN_DURATION || currentChannel == CHANNEL_START)) {
    scanComplete = true;
    isScanning = false;
    
    if (signalCount == 0) {
      noSignalsDetected = true;
    }
    
    isCooldown = true;
    cooldownStartTime = currentMillis;
    
    u8g2.clearBuffer();
    if (noSignalsDetected) {
      u8g2.drawStr(0, 10, "Scan complete. No signals.");
    } else {
      u8g2.drawStr(0, 10, "Scan complete.");
      u8g2.drawStr(0, 30, String(signalCount).c_str());
      u8g2.drawStr(30, 30, " signals found");
    }
    u8g2.sendBuffer();
    delay(1000);
  }
  
  if (isScanning && RadioA.available()) {
    if (signalCount < MAX_SIGNALS) {
      uint8_t buffer[MAX_PACKET_SIZE];
      uint8_t len = RadioA.getDynamicPayloadSize();
      
      if (len > 0 && len <= MAX_PACKET_SIZE) {
        RadioA.read(buffer, len);
        
        Signal tempSignal;
        tempSignal.length = len;
        tempSignal.channel = currentChannel;
        memcpy(tempSignal.data, buffer, len);
        
        if (!isSignalDuplicate(tempSignal)) {
          signals[signalCount].length = len;
          memcpy(signals[signalCount].data, buffer, len);
          signals[signalCount].channel = currentChannel;
          signals[signalCount].valid = true;
          signals[signalCount].timestamp = currentMillis;
          
          signalCount++;
          Serial.printf("Signal %d captured: Len=%d, Channel=%d\n", 
                        signalCount, len, currentChannel);
          
          setNeoPixelColour("green");
          delay(50);
          setNeoPixelColour("blue");
        }
      }
    }
  }
  
  if (isBroadcasting && showDetails && signals[selectedIndex].valid) {
    RadioA.setChannel(signals[selectedIndex].channel);
    RadioA.stopListening();
    RadioA.openWritingPipe(DEFAULT_PIPE_ADDRESS);
    RadioA.write(signals[selectedIndex].data, signals[selectedIndex].length);
    RadioA.startListening();
    
    setNeoPixelColour("red");
    delay(10);
    setNeoPixelColour("blue");
  }
  
  if ((currentMillis - lastDebounceUp) > DEBOUNCE_INTERVAL && digitalRead(BUTTON_PIN_UP) == LOW) {
    if (noSignalsDetected && !isCooldown) {
      startScanning();
    } else if (!showDetails && !isScanning && !isCooldown) {
      if (selectedIndex > 0) {
        selectedIndex--;
        if (selectedIndex < displayStartIndex) {
          displayStartIndex = max(0, selectedIndex);
        }
      }
    }
    lastDebounceUp = currentMillis;
  } 
  else if ((currentMillis - lastDebounceDown) > DEBOUNCE_INTERVAL && digitalRead(BUTTON_PIN_DOWN) == LOW) {
    if (noSignalsDetected && !isCooldown) {
      noSignalsDetected = false;
      scanComplete = true;
    } else if (!showDetails && !isScanning && !isCooldown) {
      if (selectedIndex < signalCount - 1) {
        selectedIndex++;
        if (selectedIndex >= displayStartIndex + 4) {
          displayStartIndex = min(signalCount - 4, selectedIndex);
        }
      }
    }
    lastDebounceDown = currentMillis;
  } 
  else if ((currentMillis - lastDebounceSelect) > DEBOUNCE_INTERVAL && digitalRead(BUTTON_PIN_SELECT) == LOW) {
    if (!isScanning && !isCooldown) {
      if (!showDetails && signalCount > 0) {
        showDetails = true;
      } else if (showDetails && !isBroadcasting) {
        isBroadcasting = true;
      } else if (showDetails && isBroadcasting) {
        isBroadcasting = false;
      }
    }
    lastDebounceSelect = currentMillis;
  } 
  else if ((currentMillis - lastDebounceBack) > DEBOUNCE_INTERVAL && digitalRead(BUTTON_PIN_BACK) == LOW) {
    if (showDetails) {
      showDetails = false;
      isBroadcasting = false;
    } else if (!isScanning && scanComplete && !isCooldown) {
      startScanning();
    }
    lastDebounceBack = currentMillis;
  }
  
  updateSignalClonerDisplay();
}

void signalclonerCleanup() {
  Serial.println("Cleaning up Signal Cloner...");
  RadioA.stopListening();
  RadioA.powerDown();
  
  pixels.setPixelColor(0, pixels.Color(0, 0, 0));
  pixels.show();
  
  u8g2.clearBuffer();
  u8g2.drawStr(0, 10, "Signal Cloner Stopped");
  u8g2.sendBuffer();
  delay(500);
}