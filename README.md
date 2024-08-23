# Tiny Key Value Store Arduino Library

A simple key-value store library using generic FS (SD/SPIFFS/FatFS/LittleFS/etc) functions for Arduino family boards.

## Getting Started

### Global (sketch beginning) part

#### AVR family boards

Use with SD shield.

```cpp
#include <SPI.h>
#include <SD.h>
#include <TinyKeyValueStore.h>

TinyKeyValueStore store = TinyKeyValueStore(SD);
```

#### ESP8266 / ESP32 boards

Use SPIFFS recommended.

```cpp
#include <SPIFFS.h>
#include <TinyKeyValueStore.h>

TinyKeyValueStore store = TinyKeyValueStore(SPIFFS);
```

Note: Don't forget to set partition scheme setting as `*** with spiffs` before uploading the sketch.

### Common part

```cpp
void setup() {
  // 1. Begin based FS class
  if (!SD.begin(SS, SPI)) Serial.println("SD Mount Failed");
  // please replace above if you use any other FS libraries
  
  // 2. Begin tiny-key-value-store
  store.begin("config.txt");
  
  // 3. Read and Write the values associated with the keys
  store.set("KEY_NAME", "hello this is VALUE");
  String val = store.get("KEY_NAME");
  
  Serial.begin(38400);
  Serial.println(val); // hello this is VALUE
}
```

## Dependency

Requires at least one generic FS extended library like SD, SPIFFS, FatFS, LittleFS, Seeed_FS, etc.

## API

### TinyKeyValueStore(fs::FS& fs_obj)

`fs_obj` : SPIFFS / FFat / LittleFS / SD / and ...

### void begin(const char* read_write_filename, const char* read_only_filename = "")

`read_write_filename` : Required. File name for storing key-value data.  
`read_only_filename` : Optional. The keys stored in this file are readable, but cannot update value, cannot create additional keys in the sketch.

### String get(const String key)

Get the value as String.

### void getCharArray(const String key, char* charArray)

Get the value as char*.

```cpp
char mes[30];
store.getCharArray("KEY_NAME", mes);
Serial.printf("Message: %s\n", mes); // Message: hello this is VALUE
```

### bool set(const String key, const String value)

Set the value.  
Returns true if the value added or updated successfully, otherwise returns false.

### bool setIfFalse(const String key, const String value)

Set the argument as value only if existing value equals to FALSE (or 0, empty string).

## License

This project is licensed under the MIT License - see the LICENSE.md file for details.