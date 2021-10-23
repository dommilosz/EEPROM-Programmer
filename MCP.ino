#define RDY_Timeout 10

void WriteMCP(MCP23017 &_mcp, uint8_t data, MCP23017Port port) {
  _mcp.writePort(port, 0x00);
  _mcp.writePort(port, data);
}

uint8_t ReadMCP(MCP23017 &_mcp, MCP23017Port port) {
  uint8_t data = _mcp.readPort(port);
  //debugln(data);
  return data;
}

void MCPTest(uint32_t pdata) {
  WriteMCP(mcp, pdata >> 24, MCP23017Port::A);
  WriteMCP(mcp, pdata >> 16, MCP23017Port::B);
  WriteMCP(mcp2, pdata >> 8, MCP23017Port::A);
  WriteMCP(mcp2, pdata, MCP23017Port::B);
}

void MCPReset() {
  WriteMCP(mcp, 0x00, MCP23017Port::A);
  WriteMCP(mcp, 0x00, MCP23017Port::B);
  WriteMCP(mcp2, 0x00, MCP23017Port::A);
  WriteMCP(mcp2, 0x00, MCP23017Port::B);
  SetPinModes();
}

uint8_t ModeToInt(uint8_t mode) {
  if (mode == OUTPUT) {
    return MCP_OUTPUT;
  } else {
    return MCP_INPUT;
  }
}

void SetPinModes() {
  eeprom.preparePins();
}



void WriteAddress(uint16_t addr) {
  eeprom.writeAddress(addr);
  debug(addr);
}

void WriteData(uint16_t data) {
  eeprom.writeData(data);
}

uint16_t ReadData() {
  return eeprom.readData();
}

bool AwaitForReady() {
  for (byte i = 0; i < RDY_Timeout; i++) {
    if (eeprom.digitalRead(eeprom.getPin(Def_RDY)) == HIGH) {
      return true;
    }
    delay(1);
  }
  return false;
}

#define BlockSize 64

void ReadEEPROM(int startAddr, int count) {
  SetPinModes();
  eeprom.digitalWrite(eeprom.getPin(Def_CE), LOW);
  eeprom.digitalWrite(eeprom.getPin(Def_OE), LOW);
  eeprom.digitalWrite(eeprom.getPin(Def_WE), HIGH);
  eeprom.IODirection(INPUT);

  if (IO_Is(IO_HTTP)) {
    server.setContentLength(count);
    server.send(200, "text/plain", "");
  }


  int bCount = ((count / BlockSize) + ((count % BlockSize) > 0));
  debug("bc: ");
  debugln(bCount);

  for (int i = 0; i < bCount; i++) {
    int blockCount = (count - (i * BlockSize));
    ReadEEPROM_Block(startAddr + (i * BlockSize), blockCount > BlockSize ? BlockSize : blockCount);
  }

  IO_Send(IO_HTTP, "");
  eeprom.digitalWrite(eeprom.getPin(Def_CE), HIGH);
  eeprom.digitalWrite(eeprom.getPin(Def_OE), HIGH);
  eeprom.digitalWrite(eeprom.getPin(Def_WE), HIGH);
}

void ReadEEPROM_Block(int startAddr, uint8_t count) {
  if (count > BlockSize) {
    Serial.print("Block too big!: ");
    return;
  }
  String msg = "";
  for (int i = 0; i < count; i++) {
    debugln();
    WriteAddress(startAddr + i);
    delayMicroseconds(20);
    String s = ReadWord();
    msg += s;
    debug(": ");
    debugln(s);
    delayMicroseconds(10);
  }
  debug("Block: ");
  debug(startAddr);
  debug(" L: ");
  debug(count);
  debug("\n");
  debugln(msg);
  //base64.encode(msg);
  //msg = String(base64.result());
  debugln(msg);
  //IO_Send(IO_BOTH, msg+",");
  IO_Send(IO_BOTH, msg);
}

String ReadWord() {
  uint16_t data = ReadData();
  uint8_t d1 = data >> 8;
  uint8_t d2 = data & 0x00FF;
  if (eeprom.IO16b) {
    return (String)((char)d1) + (String)((char)d2);
  } else {
    return (String)((char)d2);
  }
}


void WriteEEPROM(int startAddr) {
  SetPinModes();
  eeprom.digitalWrite(eeprom.getPin(Def_CE), LOW);
  eeprom.digitalWrite(eeprom.getPin(Def_OE), HIGH);
  eeprom.digitalWrite(eeprom.getPin(Def_WE), HIGH);
  eeprom.IODirection(OUTPUT);

  debugln("Write:");
  debugln(data);
  debugln(data.length());

  for (int i = 0; i < data.length(); i++) {
    debugln();
    WriteAddress(startAddr + i);
    WriteData(data[i]);
    //eeprom.digitalWrite(eeprom.getPin(Def_OE), HIGH);
    delayMicroseconds(2);
    eeprom.digitalWrite(eeprom.getPin(Def_WE), LOW);
    delayMicroseconds(1);
    eeprom.digitalWrite(eeprom.getPin(Def_WE), HIGH);
    delayMicroseconds(10);
    delay(6);
    AwaitForReady();
  }
  eeprom.digitalWrite(eeprom.getPin(Def_CE), HIGH);
  eeprom.digitalWrite(eeprom.getPin(Def_OE), HIGH);
  eeprom.digitalWrite(eeprom.getPin(Def_WE), HIGH);
}

void DumpReg(MCP23017Register reg, String name) {
  uint8_t conf = mcp.readRegister(reg);
  uint8_t conf2 = mcp2.readRegister(reg);

  JsonObject data = json.createNestedObject(name);
  data["A"] = conf;
  data["B"] = conf2;
}

void DumpRegs() {
  ClearJSON();
  
  DumpReg(MCP23017Register::IODIR_A, "IODIR_A");
  DumpReg(MCP23017Register::IODIR_B, "IODIR_B");

  DumpReg(MCP23017Register::IPOL_A, "IPOL_A");
  DumpReg(MCP23017Register::IPOL_B, "IPOL_B");

  DumpReg(MCP23017Register::GPINTEN_A, "GPINTEN_A");
  DumpReg(MCP23017Register::GPINTEN_B, "GPINTEN_B");

  DumpReg(MCP23017Register::DEFVAL_A, "DEFVAL_A");
  DumpReg(MCP23017Register::DEFVAL_B, "DEFVAL_B");

  DumpReg(MCP23017Register::INTCON_A, "INTCON_A");
  DumpReg(MCP23017Register::INTCON_B, "INTCON_B");

  DumpReg(MCP23017Register::IOCON, "IOCON");

  DumpReg(MCP23017Register::GPPU_A, "GPPU_A");
  DumpReg(MCP23017Register::GPPU_B, "GPPU_B");

  DumpReg(MCP23017Register::INTF_A, "INTF_A");
  DumpReg(MCP23017Register::INTF_B, "INTF_B");

  DumpReg(MCP23017Register::INTCAP_A, "INTCAP_A");
  DumpReg(MCP23017Register::INTCAP_B, "INTCAP_B");

  DumpReg(MCP23017Register::GPIO_A, "GPIO_A");
  DumpReg(MCP23017Register::GPIO_B, "GPIO_B");

  DumpReg(MCP23017Register::OLAT_A, "OLAT_A");
  DumpReg(MCP23017Register::OLAT_B, "OLAT_B");

  String msg = "";
  AppendJSON(msg);
  IO_SendHead(IO_BOTH,msg);
}

void MCPBenchMark(){
  long start = micros();
  WriteData(0);
  WriteData(255);
  long end = micros();
  Serial.print("W [uS]: ");
  Serial.println(end-start);

  start = micros();
  ReadData();
  end = micros();
  Serial.print("R [uS]: ");
  Serial.println(end-start);
}
