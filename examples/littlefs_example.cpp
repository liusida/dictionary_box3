#include <Arduino.h>
#include "FS.h"
#include <LittleFS.h>
#include "esp_log.h"

#define TAG "Main"

/* You only need to format LittleFS the first time you run a
   test or else use the LITTLEFS plugin to create a partition
   https://github.com/lorol/arduino-esp32littlefs-plugin

   If you test two partitions, you need to use a custom
   partition.csv file, see in the sketch folder */

//#define TWOPART

#define FORMAT_LITTLEFS_IF_FAILED true

void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  ESP_LOGI(TAG, "Listing directory: %s", dirname);

  File root = fs.open(dirname);
  if (!root) {
    ESP_LOGI(TAG, "- failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    ESP_LOGI(TAG, " - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      ESP_LOGI(TAG, "  DIR : %s", file.name());
      if (levels) {
        listDir(fs, file.path(), levels - 1);
      }
    } else {
      ESP_LOGI(TAG, "  FILE: %s\tSIZE: %u", file.name(), file.size());
    }
    file = root.openNextFile();
  }
}

void createDir(fs::FS &fs, const char *path) {
  ESP_LOGI(TAG, "Creating Dir: %s", path);
  if (fs.mkdir(path)) {
    ESP_LOGI(TAG, "Dir created");
  } else {
    ESP_LOGI(TAG, "mkdir failed");
  }
}

void removeDir(fs::FS &fs, const char *path) {
  ESP_LOGI(TAG, "Removing Dir: %s", path);
  if (fs.rmdir(path)) {
    ESP_LOGI(TAG, "Dir removed");
  } else {
    ESP_LOGI(TAG, "rmdir failed");
  }
}

void readFile(fs::FS &fs, const char *path) {
  ESP_LOGI(TAG, "Reading file: %s", path);

  File file = fs.open(path);
  if (!file || file.isDirectory()) {
    ESP_LOGI(TAG, "- failed to open file for reading");
    return;
  }

  ESP_LOGI(TAG, "- read from file:");
  while (file.available()) {
    // Note: ESP_LOGI doesn't support binary data, keeping Serial.write for file content
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char *path, const char *message) {
  ESP_LOGI(TAG, "Writing file: %s", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    ESP_LOGI(TAG, "- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    ESP_LOGI(TAG, "- file written");
  } else {
    ESP_LOGI(TAG, "- write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message) {
  ESP_LOGI(TAG, "Appending to file: %s", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    ESP_LOGI(TAG, "- failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    ESP_LOGI(TAG, "- message appended");
  } else {
    ESP_LOGI(TAG, "- append failed");
  }
  file.close();
}

void renameFile(fs::FS &fs, const char *path1, const char *path2) {
  ESP_LOGI(TAG, "Renaming file %s to %s", path1, path2);
  if (fs.rename(path1, path2)) {
    ESP_LOGI(TAG, "- file renamed");
  } else {
    ESP_LOGI(TAG, "- rename failed");
  }
}

void deleteFile(fs::FS &fs, const char *path) {
  ESP_LOGI(TAG, "Deleting file: %s", path);
  if (fs.remove(path)) {
    ESP_LOGI(TAG, "- file deleted");
  } else {
    ESP_LOGI(TAG, "- delete failed");
  }
}

// SPIFFS-like write and delete file, better use #define CONFIG_LITTLEFS_SPIFFS_COMPAT 1

void writeFile2(fs::FS &fs, const char *path, const char *message) {
  if (!fs.exists(path)) {
    if (strchr(path, '/')) {
      ESP_LOGI(TAG, "Create missing folders of: %s", path);
      char *pathStr = strdup(path);
      if (pathStr) {
        char *ptr = strchr(pathStr, '/');
        while (ptr) {
          *ptr = 0;
          fs.mkdir(pathStr);
          *ptr = '/';
          ptr = strchr(ptr + 1, '/');
        }
      }
      free(pathStr);
    }
  }

  ESP_LOGI(TAG, "Writing file to: %s", path);
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    ESP_LOGI(TAG, "- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    ESP_LOGI(TAG, "- file written");
  } else {
    ESP_LOGI(TAG, "- write failed");
  }
  file.close();
}

void deleteFile2(fs::FS &fs, const char *path) {
  ESP_LOGI(TAG, "Deleting file and empty folders on path: %s", path);

  if (fs.remove(path)) {
    ESP_LOGI(TAG, "- file deleted");
  } else {
    ESP_LOGI(TAG, "- delete failed");
  }

  char *pathStr = strdup(path);
  if (pathStr) {
    char *ptr = strrchr(pathStr, '/');
    if (ptr) {
      ESP_LOGI(TAG, "Removing all empty folders on path: %s", path);
    }
    while (ptr) {
      *ptr = 0;
      fs.rmdir(pathStr);
      ptr = strrchr(pathStr, '/');
    }
    free(pathStr);
  }
}

void testFileIO(fs::FS &fs, const char *path) {
  ESP_LOGI(TAG, "Testing file I/O with %s", path);

  static uint8_t buf[512];
  size_t len = 0;
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    ESP_LOGI(TAG, "- failed to open file for writing");
    return;
  }

  size_t i;
  ESP_LOGI(TAG, "- writing");
  uint32_t start = millis();
  for (i = 0; i < 2048; i++) {
    if ((i & 0x001F) == 0x001F) {
      ESP_LOGI(TAG, ".");
    }
    file.write(buf, 512);
  }
  ESP_LOGI(TAG, "");
  uint32_t end = millis() - start;
  ESP_LOGI(TAG, " - %u bytes written in %lu ms", 2048 * 512, end);
  file.close();

  file = fs.open(path);
  start = millis();
  end = start;
  i = 0;
  if (file && !file.isDirectory()) {
    len = file.size();
    size_t flen = len;
    start = millis();
    ESP_LOGI(TAG, "- reading");
    while (len) {
      size_t toRead = len;
      if (toRead > 512) {
        toRead = 512;
      }
      file.read(buf, toRead);
      if ((i++ & 0x001F) == 0x001F) {
        ESP_LOGI(TAG, ".");
      }
      len -= toRead;
    }
    ESP_LOGI(TAG, "");
    end = millis() - start;
    ESP_LOGI(TAG, "- %u bytes read in %lu ms", flen, end);
    file.close();
  } else {
    ESP_LOGI(TAG, "- failed to open file for reading");
  }
}

void setup() {
  // You can't mount partition to the root /, you'll get an error 258 ESP_ERR_INVALID_ARG if you end path with `/`
  if (!LittleFS.begin(true, "/data", 5, "littlefs")) {
    ESP_LOGI(TAG, "littlefs Mount Failed");
    return;
  }
  listDir(LittleFS, "/", 3);
  writeFile(LittleFS, "/hello0.txt", "World0!\r\n");
  readFile(LittleFS, "/hello0.txt");
  deleteFile(LittleFS, "/hello0.txt");
  LittleFS.end();

  ESP_LOGI(TAG, "Test complete");
}

void loop() {

  #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 4, 0)
  ESP_LOGI(TAG, ">= 5.4.0");
  #else
  ESP_LOGI(TAG, "< 5.4.0");
  #endif  
  delay(1000);
}