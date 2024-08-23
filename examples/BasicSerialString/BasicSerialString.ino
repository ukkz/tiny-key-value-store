/**
 *  Tiny Key Value Store
 *  Basic usage example: storing 2 messages alternatively
 *  Please open serial monitor and try to send a string message after uploading this sketch
 */

#ifdef ARDUINO_ARCH_ESP32
#include <FS.h>
#include <SPIFFS.h>
#include <FFat.h>
#include <LittleFS.h>
#include <SD.h>
#elif ARDUINO_ARCH_ESP8266
#include <FS.h>
#include <SD.h>
#elif __SEEED_FS__
#include <Seeed_FS.h>
#include <SD/Seeed_SD.h>
#else
#include <SPI.h>
#include <SD.h>
#endif

/* Library */
#include <TinyKeyValueStore.h>
#define MESSAGE_KEY_1     "sample_message_1"
#define MESSAGE_KEY_2     "sample_message_2"
#define STORE_FILE_NAME   "/config.txt" // filename must starts with "/"

bool store1or2 = true; // true: 1, false:2

/**
 *  Choose one FS library as arguments for storing data
 *  Note: Please check "Tools -> Partition Scheme" settings on ESP32 if happens mount failure
 *        (when using SPIFFS/FatFS/LittleFS)
 */
TinyKeyValueStore store = TinyKeyValueStore(SPIFFS);
//TinyKeyValueStore store = TinyKeyValueStore(FFat);
//TinyKeyValueStore store = TinyKeyValueStore(LittleFS);
//TinyKeyValueStore store = TinyKeyValueStore(SD);


void setup() {
  Serial.begin(38400);

  /* first, begin a FS class you chose */
  if (!SPIFFS.begin(true)) Serial.println("SPIFFS Mount Failed");
  //if (!FFat.begin(true)) Serial.println("FFat Mount Failed");
  //if (!LittleFS.begin(true)) Serial.println("LittleFS Mount Failed");
  //if (!SD.begin(SS, SPI)) Serial.println("SD Mount Failed"); // replace SS and SPI to appropriate PINs with your board

  /* second, begin TinyKeyValueStore after beginning FS class */
  store.begin(STORE_FILE_NAME);

  /* message1: get as string class */
  String sample_message_1 = store.get(MESSAGE_KEY_1);
  Serial.print("Stored message 1: "); Serial.println(sample_message_1);
  /* message2: get as char* */
  char sample_message_2[100];
  store.getCharArray(MESSAGE_KEY_2, sample_message_2);
  Serial.printf("Stored message 2: %s\n", sample_message_2);

  Serial.println("==============================");
  Serial.println("Send me something...\n");
}


void loop() {
  if (Serial.available() > 0) {
    String buf = Serial.readStringUntil('\n');
    buf.trim();
    Serial.printf("Received \"%s\"", buf.c_str());

    /* store as message 1 or 2 */
    if (store1or2) {
      Serial.println(" -> message 1");
      store.set(MESSAGE_KEY_1, buf);
    } else {
      Serial.println(" -> message 2");
      store.set(MESSAGE_KEY_2, buf);
    }
    store1or2 = !store1or2; // for next

    String sample_message_1 = store.get(MESSAGE_KEY_1);
    Serial.print("Stored message 1: "); Serial.println(sample_message_1);
    char sample_message_2[100];
    store.getCharArray(MESSAGE_KEY_2, sample_message_2);
    Serial.printf("Stored message 2: %s\n", sample_message_2);

    Serial.println("==============================");
    Serial.println("Send me something...\n");
  }
  delay(1);
}
