#include <odroid_go.h>
#include <HTTPClient.h>

#define SD_PIN_NUM_MISO 19
#define SD_PIN_NUM_MOSI 23
#define SD_PIN_NUM_CLK  18
#define SD_PIN_NUM_CS 22

String githubSources[100];
int sourceCount = 0;
bool httpError = false;

void deleteFile(fs::FS &fs, const char * path) {
  Serial.printf("Deleting file: %s\n", path);
  GO.lcd.printf("Deleting file: %s\n", path);
  if (fs.remove(path)) {
    Serial.println("File deleted");
    GO.lcd.println("File deleted");
  } else {
    Serial.println("Delete failed");
    GO.lcd.println("Delete failed");
  }
}

void listDir(fs::FS &fs, const char * dirname, const char * fileToRemove, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);
  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), fileToRemove, levels - 1);
      }
    } else {
      String fileName = file.name();
      if (fileName.indexOf(fileToRemove) > 0) {
        GO.lcd.println(file.name());
        deleteFile(SD, fileName.c_str());
      }
    }
    file = root.openNextFile();
  }
}

String escapeParameter(String param) {
  param.replace("+", " ");
  param.replace("%21", "!");
  param.replace("%23", "#");
  param.replace("%24", "$");
  param.replace("%26", "&");
  param.replace("%27", "'");
  param.replace("%28", "(");
  param.replace("%29", ")");
  param.replace("%2A", "*");
  param.replace("%2B", "+");
  param.replace("%2C", ",");
  param.replace("%2F", "/");
  param.replace("%3A", ":");
  param.replace("%3B", ";");
  param.replace("%3D", "=");
  param.replace("%3F", "?");
  param.replace("%40", "@");
  param.replace("%5B", "[");
  param.replace("%5D", "]");
  param.replace("&amp;", "&");
  return param;
}

String midString(String str, String start, String finish) {
  int locStart = str.indexOf(start);
  if (locStart == -1) return "";
  locStart += start.length();
  int locFinish = str.indexOf(finish, locStart);
  if (locFinish == -1) return "";
  return str.substring(locStart, locFinish);
}


void readFileSize(fs::FS &fs, const char * path) {
  Serial.printf("Reading file size: %s\n", path);
  GO.lcd.printf("Reading file size: %s\n", path);
  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    GO.lcd.println("Failed to open file for reading");
    return;
  }
  int fileSize = file.size();
  file.close();
  if (fileSize <= 0) {
    GO.lcd.println("Delete file because <= 0 Byte. (Corrupted)");
    deleteFile(SD, path);
  } else {
    GO.lcd.print("File size:");
    GO.lcd.println(fileSize);
  }
}

void setup() {
  GO.begin();
  pinMode(25, OUTPUT);
  digitalWrite(25, LOW);
  GO.lcd.setTextWrap(false);

  Serial.println();
  Serial.println();

  SPI.begin(SD_PIN_NUM_MISO, SD_PIN_NUM_MOSI, SD_PIN_NUM_CLK, SD_PIN_NUM_CS);
  if (!SD.begin(SD_PIN_NUM_CS)) {
    Serial.println("Card Mount Failed");
    GO.lcd.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    GO.lcd.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  GO.lcd.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
    GO.lcd.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
    GO.lcd.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
    GO.lcd.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
    GO.lcd.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  GO.lcd.printf("SD Card Size: %lluMB\n", cardSize);

  String path = "/URLLIST.TXT";
  Serial.print("Reading file: ");
  Serial.println(path);
  GO.lcd.print("Reading file: ");
  GO.lcd.println(path);
  File sourceFile = SD.open(path);
  if (!sourceFile) {
    Serial.println("Failed to open file for reading");
    GO.lcd.println("Failed to open file for reading");
    return;
  }
  Serial.println("Read from file: ");
  GO.lcd.println("Read from file: ");
  while (sourceFile.available()) {
    githubSources[sourceCount] = sourceFile.readStringUntil('\n');
    githubSources[sourceCount].replace("\r", "");
    sourceCount++;
  }
  sourceFile.close();

  for (int i = 0; i < sourceCount; i++) {
    Serial.println(githubSources[i]);
    GO.lcd.println(githubSources[i]);
  }
  Serial.println();
  GO.lcd.println();


  String WifiSSID = "";
  String WifiPSK = "";

  path = "/WIFI.TXT";
  Serial.print("Reading file: ");
  Serial.println(path);
  GO.lcd.print("Reading file: ");
  GO.lcd.println(path);
  File wifiFile = SD.open(path);
  if (!wifiFile) {
    Serial.println("Failed to open file for reading");
    GO.lcd.println("Failed to open file for reading");
    return;
  }
  Serial.println("Read from file: ");
  GO.lcd.println("Read from file: ");
  while (wifiFile.available()) {
    WifiSSID = wifiFile.readStringUntil('\n');
    WifiSSID.replace("\r", "");
    WifiPSK = wifiFile.readStringUntil('\n');
    WifiPSK.replace("\r", "");
    break;
  }
  wifiFile.close();
  Serial.print("SSID: ");
  Serial.println(WifiSSID);
  Serial.print("PSK: ");
  Serial.println(WifiPSK);
  GO.lcd.print("SSID: '");
  GO.lcd.print(WifiSSID);
  GO.lcd.println("'");
  GO.lcd.print("PSK: '");
  GO.lcd.print(WifiPSK);
  GO.lcd.println("'");
  Serial.println();

  //delete old wifi Credentials
  WiFi.disconnect();
  
  WiFi.begin(WifiSSID.c_str(), WifiPSK.c_str());
  Serial.println("MAC:");
  Serial.println(WiFi.macAddress());
  Serial.print("Connecting Wifi");
  GO.lcd.print("Connecting Wifi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    GO.lcd.print(".");
  }
  GO.lcd.println();
  Serial.println();
  delay(3000);


  for (int i = 0; i < sourceCount; i++) {
    // wait for WiFi connection
    if ((WiFi.status()  == WL_CONNECTED)) {
      httpError = false;
      HTTPClient http;
      String latestURL = "", releaseStr = "", fileName = "";

      GO.lcd.clearDisplay();
      GO.lcd.setCursor(0, 0);

      Serial.print("Get Latest FW link from Github API\n");
      Serial.println(githubSources[i]);
      GO.lcd.print("Get Latest FW link from Github API\n");
      GO.lcd.println(githubSources[i]);
      http.begin(githubSources[i]); //HTTP
      int httpCode = http.GET();
      if (httpCode > 0) {
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        GO.lcd.printf("[HTTP] GET... code: %d\n", httpCode);
        if (httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          releaseStr = midString( payload, "\"tag_name\":\"", "\"" );
          latestURL = midString( payload, "\"browser_download_url\":\"", "\"" );

          char *str;
          char sz[256];
          char *p = sz;
          latestURL.toCharArray(sz, 256);
          while ((str = strtok_r(p, "/", &p)) != NULL)  // Don't use \n here it fails
          {
            fileName = str;
          }

          Serial.println(releaseStr);
          Serial.println(fileName);
          Serial.println(latestURL);
          GO.lcd.println(releaseStr);
          GO.lcd.println(fileName);
          GO.lcd.println(latestURL);
        } else if (httpCode == HTTP_CODE_FORBIDDEN ) {
          Serial.println("You reached the rate_limit from Github API, please wait 30 Minutes");
          GO.lcd.println("You reached the rate_limit from Github API, please wait 30 Minutes");
          httpError = true;
        } else {
          httpError = true;
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        GO.lcd.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        httpError = true;
      }
      http.end();

      String path = "/odroid/firmware/" + releaseStr + "_" + fileName;
      //Check if file is >0 Byte if not delete it
      readFileSize(SD, path.c_str());
      //remove old versions
      listDir(SD, "/odroid/firmware/", fileName.c_str(), 0);
      if (!SD.exists(path)) {
        if (!httpError) {
          Serial.print("Get Latest FW URL\n");
          GO.lcd.print("Get Latest FW URL\n");
          http.begin(latestURL);
          int httpCode = http.GET();
          if (httpCode > 0) {
            Serial.printf("[HTTP] GET... FW: %d\n", httpCode);
            GO.lcd.printf("[HTTP] GET... FW: %d\n", httpCode);
            String payload = http.getString();
            if (httpCode == HTTP_CODE_FOUND || httpCode == HTTP_CODE_OK) {
              latestURL = escapeParameter(midString( payload, "You are being <a href=\"", "\">redirected" ));
              Serial.println("URL-Decode:");
              Serial.println(latestURL);
              GO.lcd.println("URL-Decode:");
              GO.lcd.println(latestURL);
            } else {
              httpError = true;
            }
          } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
            GO.lcd.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
            httpError = true;
          }
          http.end();


          if (!httpError) {
            Serial.print("Download latest FW from Github.\n");
            GO.lcd.print("Download latest FW from Github.\n");
            http.begin(latestURL);
            int httpCode = http.GET();
            if (httpCode > 0) {
              Serial.printf("[HTTP] GET... code: %d\n", httpCode);
              GO.lcd.printf("[HTTP] GET... code: %d\n", httpCode);
              int len = http.getSize();
              uint8_t buff[2048] = { 0 };
              WiFiClient * stream = http.getStreamPtr();
              Serial.print("Downloading FW to SD Card:");
              Serial.println(path);
              GO.lcd.print("Downloading FW to SD Card:");
              GO.lcd.println(path);
              File file = SD.open(path, FILE_WRITE);
              if (!file) {
                Serial.println("Failed to open file for writing");
                GO.lcd.println("Failed to open file for writing");
              } else {
                Serial.print("Size:");
                Serial.println(len);
                GO.lcd.print("Size:");
                GO.lcd.println(len);
                int lenLoad = len;
                // read all data from server
                while (http.connected() && (len > 0 || len == -1)) {
                  // get available data size
                  size_t size = stream->available();
                  if (size) {
                    // read up to 128 byte
                    int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                    if (!file.write(buff, c)) {
                      Serial.println("Write failed");
                      GO.lcd.println("Write failed");
                      break;
                    }
                    Serial.print("Loading:");
                    Serial.print(abs(lenLoad - len));
                    Serial.print(" / ");
                    Serial.println(len);

                    GO.lcd.fillRect(0, 190, TFT_HEIGHT , 30, BLACK);
                    GO.lcd.setCursor(0, 200);
                    GO.lcd.print("Loading:");
                    GO.lcd.print(abs(lenLoad - len));
                    GO.lcd.print(" / ");
                    GO.lcd.println(lenLoad);
                    if (len > 0) {
                      len -= c;
                    }
                  }
                }
              }
              file.close();
              Serial.print("[HTTP] connection closed or Download Finished.\n");
              GO.lcd.print("[HTTP] connection closed or Download Finished.\n");
            } else {
              Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
              GO.lcd.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
            }
            http.end();
          }
        }

        if (httpError) {
          Serial.println("Error downloading Firmware!");
          GO.lcd.println("Error downloading Firmware!");
        }
      } else
      {
        Serial.println("File is already up to date!");
        GO.lcd.println("File is already up to date!");
      }
    }
    else
    {
      Serial.println("No Connection!");
      GO.lcd.println("No Connection!");
    }

    Serial.println("");
    Serial.println("");
    Serial.println("");
    GO.lcd.println("");
    delay(1000);
  }
  Serial.println("Everything is Updated!");

  GO.lcd.clearDisplay();
  GO.lcd.setCursor(0, 0);
  GO.lcd.println("Everything is Updated!");
}

void loop() {
  GO.update();
}
