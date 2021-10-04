int CharsToInt(uint8_t a, uint8_t b) {
  int countI = b;
  countI |= a << 8;
  return countI;
}

uint32_t CharsToInt32(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
  uint32_t countI = d;
  countI |= c << 8;
  countI |= b << 16;
  countI |= a << 24;
  return countI;
}

uint32_t GetInt32(String argname) {
  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == argname) {
      char d[4];
      memcpy(&d, &server.arg(i), 4);
      uint16_t i1 = hex2int(d);
      memcpy(&d, &server.arg(i) + 4, 4);
      uint16_t i2 = hex2int(d);

      return (i1 << 16) | i2;
    }
  }
}

uint16_t GetInt16(String argname) {
  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == argname) {
      char d[4];
      memcpy(&d, &server.arg(i), 4);
      return hex2int(d);
    }
  }
}

uint8_t GetInt8(String argname) {
  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == argname) {
      char d[2];
      memcpy(&d, &server.arg(i), 2);
      return hex2int(d);
    }
  }
}

uint16_t hex2int(const char *hex)
{
  uint16_t value;  // unsigned to avoid signed overflow
  for (value = 0; *hex; hex++) {
    value <<= 4;
    if (*hex >= '0' && *hex <= '9')
      value |= *hex - '0';
    else if (*hex >= 'A' && *hex <= 'F')
      value |= *hex - 'A' + 10;
    else if (*hex >= 'a' && *hex <= 'f')
      value |= *hex - 'a' + 10;
    else
      break;  // stop at first non-hex digit
  }
  return value;
}

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

uint16_t CreatePinID(uint8_t type, uint8_t id){
  return ((type << 8) | id);
}

void listFiles() {
  if(DEBUG == 0)return;
  debugln("LittleFS files:");
  Dir root = LittleFS.openDir("/");
  while (root.next()) {
    File file = root.openFile("r");
    debug("  FILE: ");
    debug(root.fileName());
    debug("  SIZE: ");
    debug(file.size());
    file.close();  }
}
