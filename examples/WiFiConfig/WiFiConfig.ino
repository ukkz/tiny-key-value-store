/**
 *  Tiny Key Value Store
 *  Advanced usage example: Edit WiFi info with your smartphone and keep the configuration
 *  ================================================================================
 *  Note: THIS SKETCH IS DESIGNED ONLY FOR M5STACK FAMILY DEVICES WITH DISPLAY
 *  ================================================================================
 *  1. Write this sketch and boot your device
 *  2. Please press any button or touch display just after boot up the device to be config mode
 *  3. Scan QR code by your smartphone to join the device's wifi access point
 *  4. Wait some seconds or following the smartphone's instruction to access the WiFi config page (Captive Portal Web)
 *  5. Please input WiFi config information and NTP hostname
 *  6. Tap "Update & Reboot" button to reboot your device
 *  7. If all information is correct, your device will be connected WiFi and got NTP server time
 */

/* M5 */
#include <M5Unified.h>
#include <FS.h>
#include <SPIFFS.h>
#include <FFat.h>
#include <LittleFS.h>
#include <SD.h>

/* WiFi */
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <WiFiAP.h>
#include <DNSServer.h>
WiFiClientSecure client;

/* Library */
#include <TinyKeyValueStore.h>
#define STORE_FILE_NAME   "/config.txt" // filename must starts with "/"

/* Keys & Default values */
#define KEY_WIFI_SSID   "wifi_ssid"
#define KEY_WIFI_PSWD   "wifi_passwd"
#define KEY_NTPD_HOST   "ntp_host"
char ssid[64];
char pswd[64];
char host[128];

/* Mode flag (Normal or Config) */
bool config_mode = false;
/* Config Web page HTML */
const char* config_html_template =
#include "config.html.h"
;
String configHTML = String(""); // create later from template with some vars
/* Config Web Server */
const char* config_ssid = "TinyKeyValueStoreDevice";
WebServer configServer(80);
const byte DNS_PORT = 53;
IPAddress dnsIP(8, 8, 4, 4);
DNSServer dnsServer;

/**
 *  Choose one FS library as arguments for storing data
 *  Note: Please check "Tools -> Partition Scheme" settings on ESP32 if happens mount failure
 *        (when using SPIFFS/FatFS/LittleFS)
 */
TinyKeyValueStore store = TinyKeyValueStore(SPIFFS);
//TinyKeyValueStore store = TinyKeyValueStore(FFat);
//TinyKeyValueStore store = TinyKeyValueStore(LittleFS);
//TinyKeyValueStore store = TinyKeyValueStore(SD);

/* Reading config vars and create HTML */
void configLoad(bool updated = false) {
  store.getCharArray(KEY_WIFI_SSID, ssid);
  store.getCharArray(KEY_WIFI_PSWD, pswd);
  store.getCharArray(KEY_NTPD_HOST, host);
  char buf[3000];
  const char* updated_notice = updated ? "Updated" : "";
  sprintf(buf, config_html_template, updated_notice, ssid, pswd, host);
  configHTML = String(buf);
}

/* Handler function for accessing the config web server */
void configWeb() {
  if (configServer.method() == HTTP_POST) {
    /* POST access: updating vars */
    if (configServer.arg(KEY_WIFI_SSID).length() != 0) store.set(KEY_WIFI_SSID, configServer.arg(KEY_WIFI_SSID));
    if (configServer.arg(KEY_WIFI_PSWD).length() != 0) store.set(KEY_WIFI_PSWD, configServer.arg(KEY_WIFI_PSWD));
    if (configServer.arg(KEY_NTPD_HOST).length() != 0) store.set(KEY_NTPD_HOST, configServer.arg(KEY_NTPD_HOST));
    if (configServer.arg("reboot").equals("yes")) ESP.restart(); // when reboot button clicked
    configLoad(true);  // updated
  } else {
    /* GET access: only show current config values */
    configLoad(false); // not updated
  }
  configServer.send(200, "text/html", configHTML);
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setTextColor(0xFFFFFFU, 0x000000U);
  M5.Display.setTextFont(2);

  /* first, begin a FS class you chose */
  if (!SPIFFS.begin(true)) Serial.println("SPIFFS Mount Failed");
  //if (!FFat.begin(true)) Serial.println("FFat Mount Failed");
  //if (!LittleFS.begin(true)) Serial.println("LittleFS Mount Failed");
  //if (!SD.begin(SS, SPI)) Serial.println("SD Mount Failed"); // replace SS and SPI to appropriate PINs with your board

  /* second, begin TinyKeyValueStore after beginning FS class */
  store.begin(STORE_FILE_NAME);

  /* reading char variables from store */
  store.getCharArray(KEY_WIFI_SSID, ssid);
  store.getCharArray(KEY_WIFI_PSWD, pswd);
  store.getCharArray(KEY_NTPD_HOST, host);

  /* Check pressing BtnA or not when M5 just booting up  */
  M5.Display.printf("To configure WiFi,\nPlease press any button or touch display ...");
  for (int i=0; i<500; i++) {
    delay(10);
    M5.update();
    if (M5.BtnA.isPressed() || M5.BtnB.isPressed() || M5.BtnC.isPressed() || M5.Touch.getCount() > 0) {
      config_mode = true;
      break;
    }
  }
  M5.Display.clear();
  M5.Display.setCursor(0, 0);
  
  if (config_mode) {
    /* Pressed: WiFi config mode - Start WiFi AP and Web server */
    WiFi.softAP(config_ssid);
    WiFi.softAPConfig(dnsIP, dnsIP, IPAddress(255, 255, 255, 0));
    dnsServer.start(DNS_PORT, "*", dnsIP);
    /* Serve as CaptivePortal */
    configServer.onNotFound(configWeb);
    configServer.begin();
    /**
     *  show ZXing's style WiFi connection QR
     *  see: https://zxing.appspot.com/generator
     */
    String qrStr = "WIFI:S:" + String(config_ssid) + ";;";
    M5.Display.qrcode(qrStr);
  } else {
    /* Not: Normal mode - NTP Client */
    WiFi.mode(WIFI_STA); WiFi.disconnect();
    M5.Display.printf("WiFi SSID: %s\n", ssid);
    WiFi.begin(ssid, pswd);
    while (WiFi.status() != WL_CONNECTED) {
      if (millis() >= 60000UL) ESP.restart(); // reboot when couldnt connect within 1 min
      M5.Display.print("."); delay(1000);
    }
    M5.Display.printf("\nConnected\nIP: %s\n", WiFi.localIP().toString().c_str());
    /* Get time from NTP */
    M5.Display.printf("\nNTP Host: %s\n", host);
    configTime(0, 0, host);
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      M5.Display.printf("Time: failed to get\n");
    } else {
      M5.Display.printf("%04d-%02d-%02d %02d:%02d:%02d" 
                        ,timeinfo.tm_year + 1900
                        ,timeinfo.tm_mon + 1
                        ,timeinfo.tm_mday
                        ,timeinfo.tm_hour
                        ,timeinfo.tm_min
                        ,timeinfo.tm_sec
                        );
    }
  }
}


void loop() {
  /* Keep handling CaptivePortal when config mode */
  if (config_mode) {
    dnsServer.processNextRequest();
    configServer.handleClient();
  }
  delay(1);
}
