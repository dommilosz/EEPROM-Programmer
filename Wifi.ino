//#define STASSID "Przylesie2"
//#define STAPSK  "Miloszek5#"

#define APSSID "EEPROM_PROGRAMMER - Fallback"
#define APPSK  "12345678"

String DefSSID;
String DefPASS;

String DefASSID = APSSID;
String DefAPSK = APPSK;

DynamicJsonDocument wifi_json(256);

#define CONNECT_TIMEOUT 15000

void WifiConnect() {
  WiFi.softAPdisconnect();
  if (WiFi.status() == WL_CONNECTED) {
    WiFi.disconnect();
    for (byte i = 0; i < 5; i++) {
      if (WiFi.status() != WL_CONNECTED)break;
      delay(1000);
    }
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(DefSSID, DefPASS);

  long startts = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    long ts = millis();

    if (ts - startts > CONNECT_TIMEOUT) {
      if (WiFi.status() != WL_CONNECTED) {
        WifiOnTimedOut();
        return;
      }
    }
  }
  WifiOnSuccess();
}

void WifiOnSuccess() {
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(DefSSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void WifiOnTimedOut() {
  Serial.println("");
  Serial.print("Failed to connect: ");
  Serial.println(DefSSID);
  Serial.println(WiFi.status());

  WiFi.mode(WIFI_AP);
  WiFi.softAP(DefASSID, DefAPSK);

  Serial.println("Creating AP: ");
  Serial.println(DefASSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
}

void WifiReadConfig() {
  File file = LittleFS.open("/wifi", "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    listFiles();
    return;
  }
  String confJSON = "";
  while (file.available()) {
    confJSON += char(file.read());
  }

  ReadJSON(confJSON, json);

  TransferPropertyMsg(wifi_json, json, "ssid", true);
  TransferPropertyMsg(wifi_json, json, "pass", true);
  TransferPropertyMsg(wifi_json, json, "ap_ssid", true);
  TransferPropertyMsg(wifi_json, json, "ap_pass", true);

  WifiVarsFromJSON();

  debugln(confJSON);
}

void WifiWriteConfig() {
  File file = LittleFS.open("/wifi", "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  String confJSON = "";
  AppendJSON(confJSON, wifi_json);
  file.print(confJSON);
  debugln(confJSON);
}

void WifiResetConfig() {
  wifi_json["ssid"] = "";
  wifi_json["pass"] = "";
  wifi_json["ap_ssid"] = APSSID;
  wifi_json["ap_pass"] = APPSK;

  WifiVarsFromJSON();
  WifiWriteConfig();
}

void WifiVarsFromJSON() {
  DefSSID = String(wifi_json["ssid"]);
  DefPASS = String(wifi_json["pass"]);

  DefASSID = String(wifi_json["ap_ssid"]);
  DefAPSK = String(wifi_json["ap_pass"]);
}

void GetWifiConfigSV() {
  ReadJSONSV(json);

  TransferPropertyMsg(wifi_json, json, "ssid", true);
  TransferPropertyMsg(wifi_json, json, "pass", true);
  TransferPropertyMsg(wifi_json, json, "ap_ssid", true);
  TransferPropertyMsg(wifi_json, json, "ap_pass", true);

  WifiVarsFromJSON();
  WifiWriteConfig();
}
