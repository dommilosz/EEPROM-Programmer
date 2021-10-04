void handleRoot() {
  File file = LittleFS.open("/index.html", "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    listFiles();
    return;
  }
  String index = "";
  while (file.available()) {
    index += char(file.read());
  }
  server.send(200, "text/html", index);
}

void handleRead() {
  if (!CheckPost() && IO_Is(IO_HTTP))return;

  IO_ReadJSON(IO_BOTH, json);
  int address = json["address"];
  int count   = json["count"];

  debugln("Read: Addr:");
  debugln(address);
  debugln("C: ");
  debugln(count);

  ReadEEPROM(address, count);
}

void handleWrite() {
  if (!CheckPost() && IO_Is(IO_HTTP))return;

  IO_ReadJSON(IO_BOTH, json);
  int address = json["address"];
  data   = String(json["data"]);

  debugln("Read: Addr:");
  debugln(address);
  debugln("C:");
  debugln(data);

  WriteEEPROM(address);
  IO_SendHead(IO_BOTH, "done");
}

void handleJSON() {
  if (!CheckPost() && IO_Is(IO_HTTP))return;

  data = "POST json was:\n";
  IO_ReadJSON(IO_BOTH, json);
  AppendJSON(data);
  IO_SendHead(IO_BOTH, data);
}

void handleMCPDWrite() {
  if (!CheckPost() && IO_Is(IO_HTTP))return;

  IO_ReadJSON(IO_BOTH, json);
  int pin = json["pin"];
  int state   = json["state"];

  eeprom.digitalWrite(pin, state);
  IO_SendHead(IO_BOTH, "done");
}

void handleMCPWrite() {
  if (!CheckPost() && IO_Is(IO_HTTP))return;
  MCP23017Register reg;

  IO_ReadJSON(IO_BOTH, json);
  int data       = json["data"];
  int regi       = json["reg"];
  int mcpindex   = json["i"];
  reg = (MCP23017Register)regi;

  if (mcpindex == 0) {
    mcp.writeRegister(reg, data);
  }
  if (mcpindex == 1) {
    mcp2.writeRegister(reg, data);
  }

  IO_SendHead(IO_BOTH, "done");
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void handleMcpDump() {
  if (IO_Is(IO_HTTP)) {
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/plain", "");
  }

  DumpRegs();

  if (IO_Is(IO_HTTP)) {
    server.sendContent("");
  }
}

void handlePinDefConfiguration() {
  if (server.method() == HTTP_GET) {
    SendMappings();
    return;
  }
  if (server.method() == HTTP_POST) {
    LoadMappings();
    SendMappings();
    return;
  }
}

void handleAIOWrite() {
  if (!CheckPost() && IO_Is(IO_HTTP))return;
  IO_ReadJSON(IO_BOTH, json);

  int address = json["address"];
  int data    = json["data"];

  if (address >= 0) {
    eeprom.writeAddress(address);
  }
  if (data >= 0) {
    eeprom.IODirection(OUTPUT);
    eeprom.writeData(data);
  }

  IO_SendHead(IO_BOTH, "done");
}

void handleWifiConfig() {
  if (!CheckPost() && IO_Is(IO_HTTP))return;
  IO_SendHead(IO_BOTH, "done");
  GetWifiConfigSV();
  WifiConnect();
}
