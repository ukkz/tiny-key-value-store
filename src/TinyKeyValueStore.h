#pragma once
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

class TinyKeyValueStore {
  private:
    fs::FS& fs;
    const char* rofile;
    const char* rwfile;
    const String KEYVALUE_SEPARATOR = ":";
    const char ENTRY_SEPARATOR = '\n';
    bool exist(const String filename, const String key) {
      bool result = false;
      File f = fs.open(filename);
      while (true) {
        // read 1 line / 1 loop
        String line = f.readStringUntil(ENTRY_SEPARATOR);
        // break if no any lines
        if (!line.length()) break;
        // looking up the key string
        int i = line.indexOf(key + KEYVALUE_SEPARATOR);
        if (i == 0) result = true;
      }
      f.close();
      return result;
    }
    String read(const String filename, const String key) {
      String val = String("");
      File f = fs.open(filename);
      while (true) {
        // read 1 line / 1 loop
        String line = f.readStringUntil(ENTRY_SEPARATOR);
        // break if no any lines
        if (!line.length()) break;
        // looking up the key string
        int i = line.indexOf(key + KEYVALUE_SEPARATOR);
        if (i == 0) {
          // HEAD index of the value
          int s = line.indexOf(KEYVALUE_SEPARATOR) + 1;
          // get to TAIL of the value
          val = line.substring(s);
          break;
        }
      }
      val.trim();
      f.close();
      return val;
    }
    
  public:
    TinyKeyValueStore(fs::FS& fs_obj) : fs(fs_obj) {}
    void begin(const char* read_write_filename, const char* read_only_filename = "") {
      rwfile = read_write_filename;
      rofile = read_only_filename;
      // Create file if not exists (ReadWriteFile only)
      if (!fs.exists(rwfile)) {
        // WIP: output debug info
        //Serial.printf("File \"%s\" not found - created\n", rwfile);
        File f = fs.open(rwfile, "w");
        f.write(ENTRY_SEPARATOR);
        f.close();
      }
    }
    String get(const String key) {
      // return the value from ReadOnlyFile preferentially (in case duplicate key written in ReadWriteFile)
      if (!String(rofile).equals("")) {
        String ro = read(rofile, key);
        if (!ro.equals("")) return ro;
      }
      // return the value from ReadWriteFile in case ReadOnlyFile is not set or the key is not exist
      String rw = read(rwfile, key);
      if (!rw.equals("")) return rw;
      return "";
    }
    void getCharArray(const String key, char* charArray) {
      // use if need char* instead of this.get().c_str()
      String val = get(key);
      val.toCharArray(charArray, val.length()+1);
    }
    bool set(const String key, const String value) {
      // Reading the entire of ReadWriteFile except for the lines to be updated
      String buf = String("");
      File f = fs.open(rwfile);
      while (true) {
        // read 1 line / 1 loop
        String line = f.readStringUntil(ENTRY_SEPARATOR);
        // break if no any lines
        if (!line.length()) break;
        // looking up the key string
        if (line.indexOf(key) == -1) {
          // not: line -> buffer
          buf = buf + line + ENTRY_SEPARATOR;
        } else {
          // exist: ignore current line and add new value to buffer with this key
          buf = buf + key + KEYVALUE_SEPARATOR + value + ENTRY_SEPARATOR;
        }
      }
      f.close();
      // add new key
      if (buf.indexOf(key + KEYVALUE_SEPARATOR) == -1) buf = buf + key + KEYVALUE_SEPARATOR + value + ENTRY_SEPARATOR;
      // Delete ReadWriteFile
      bool d = fs.remove(rwfile);
      if (!d) {
        //Serial.printf("Couldn't delete: %s\n", rwfile);
        return false;
      }
      // Re-generate ReadWriteFile and write the buffer
      File rw = fs.open(rwfile, "w");
      if (!rw) {
        //Serial.printf("Couldn't open: %s\n", rwfile);
        return false;
      }
      rw.print(buf);
      rw.close();
      return true;
    }
    bool setIfFalse(const String key, const String value) {
      // set the argument as value only if existing value equals with FALSE (or 0, empty string)
      String target_value = read(rwfile, key);
      if (target_value.equals("") || target_value.toInt() == 0 || target_value.toFloat() == 0.0) {
        return set(key, value);
      } else {
        return false;
      }
    }
};