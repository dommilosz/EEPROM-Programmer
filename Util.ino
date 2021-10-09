bool CheckPost() {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    return false;
  }
  return true;
}

uint8_t ReverseUint8(uint8_t b) {
  b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
  b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
  b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
  return b;
}

uint16_t CreatePinID(uint8_t type, uint8_t id) {
  return ((type << 8) | id);
}

void listFiles() {
  if (DEBUG == 0)return;
  debugln("LittleFS files:");
  Dir root = LittleFS.openDir("/");
  while (root.next()) {
    File file = root.openFile("r");
    debug("  FILE: ");
    debug(root.fileName());
    debug("  SIZE: ");
    debug(file.size());
    file.close();
  }
}
