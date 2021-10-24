#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Wire.h>
#include <MCP23017.h>
#include <ArduinoJson.h>
#include "LittleFS.h"
#include <rBase64.h>
#define D11 9
#define D12 10

#define Def_GND  0x00 //OUT
#define Def_WE   0x01 //OUT
#define Def_CE   0x02 //OUT
#define Def_OE   0x03 //OUT
#define Def_A    0x04 //OUT
#define Def_IO   0x05 //IN/OUT
#define Def_SDA  0x06 //IN/OUT
#define Def_CLK  0x07 //OUT
#define Def_MISO 0x08 //IN
#define Def_MOSI 0x09 //OUT
#define Def_CS   0x0A //OUT
#define Def_NC   0x0B //IN
#define Def_RDY  0x0C //IN

#define MCP_OUTPUT 0b00000000
#define MCP_INPUT  0b11111111

#define DEBUG 0
#define DEBUG_VERBOSE 0
bool debug_ovr = true;

#if DEBUG == 1
#define debug(x) if(debug_ovr) Serial.print(x)
#define debugln(x) if(debug_ovr) Serial.println(x)
#define debugBIN(x) if(debug_ovr) Serial.print(x,BIN)
#define debuglnBIN(x) if(debug_ovr) Serial.println(x,BIN)
#define debugHEX(x) if(debug_ovr) Serial.print(x,HEX)
#define debuglnHEX(x) if(debug_ovr) Serial.println(x,HEX)
#else
#define debug(x)
#define debugln(x)
#define debugBIN(x)
#define debuglnBIN(x)
#define debugHEX(x)
#define debuglnHEX(x)
#endif

#if DEBUG_VERBOSE
#define Vdebug(x) debug(x)
#define Vdebugln(x) debugln(x)
#define VdebugBIN(x) debugBIN(x,BIN)
#else
#define Vdebug(x)
#define Vdebugln(x)
#define VdebugBIN(x)
#endif

#define IO_HTTP 0
#define IO_SERIAL 1
#define IO_BOTH 2

MCP23017 mcp = MCP23017(0x20);
MCP23017 mcp2 = MCP23017(0x21);
#define KHz(x) 1000*x
#define MHz(x) KHz(1000*x)
#define CreatePinID(type,id) ((type << 8) | id)

#define I2C_SPEED KHz(666)

DynamicJsonDocument json(2048);
DynamicJsonDocument wifi_json(256);
rBase64generic<8192> base64;

void MCPDigitalWrite(MCP23017 &_mcp, uint8_t pin, bool state) {
  _mcp.digitalWrite(pin, state);
  //DumpRegs();
}

class EEPROMDef {
  public:
    uint16_t mapping[32];
    uint8_t  mapping_c[13][16];
    uint32_t pinstates = 0x00000000;
    uint32_t pinstates_mode = 0xFFFFFFFF;
    uint8_t  pinstates_read[4];

    void LoadMappings(uint16_t new_mappings[]) {
      memcpy(mapping, new_mappings, sizeof(mapping));
      CompileMappings();
    }

    void CompileMappings() {
      for (byte i = 0; i < 13; i++) {
        for (byte j = 0; j < 16; j++) {
          mapping_c[i][j] = getPin(i, j, false);
        }
      }
    }

    void SetupDefaultMappings() {
      uint16_t mapping2[32] = {0x0402, 0x0403, 0x0404, 0x0405, 0x0406, 0x0407, 0x040C, 0x0C00, 0x0502, 0x0501, 0x0500, 0x0400, 0x0401, 0x0000, 0x0B00, 0x0100, 0x040B, 0x0409, 0x0408, 0x0B00, 0x0B00, 0x0B00, 0x0B00, 0x0B00, 0x0503, 0x0504, 0x0505, 0x0506, 0x0507, 0x0200, 0x040A, 0x0300};
      LoadMappings(mapping2);
    }

    uint8_t getPin(uint8_t type, uint8_t id = 0x00, bool compiled = true) {
      if (compiled) {
        return mapping_c[type][id];
      }

      uint16_t looking = CreatePinID(type, id);
      for (byte i = 0; i < 32; i++) {
        if (mapping[i] == looking) {
          return i;
        }
      }
      return -1;
    }

    void pinstateWrite(uint8_t pin, bool state) {
      if (state) {
        pinstates |= (1 << pin);
      } else {
        pinstates &= ~(1 << pin);
      }
      VdebugBIN(pinstates);
    }

    void pinstateWriteMode(uint8_t pin, bool state) {
      if (!state) {
        pinstates_mode |= (1 << pin);
      } else {
        pinstates_mode &= ~(1 << pin);
      }
    }

    void applyState() {
      uint8_t RA1 = pinstates;
      uint8_t RB1 = pinstates >> 8;
      uint8_t RA2 = pinstates >> 16;
      uint8_t RB2 = pinstates >> 24;

      RA1 = ReverseUint8(RA1);
      //RB1 = ReverseUint8(RB1);
      RA2 = ReverseUint8(RA2);
      //RB2 = ReverseUint8(RB2);

      mcp.writeRegister(MCP23017Register::GPIO_A, RA1, RB1);
      mcp2.writeRegister(MCP23017Register::GPIO_A, RA2, RB2);
    }

    void applyModeState() {
      uint8_t RA1 = pinstates_mode;
      uint8_t RB1 = pinstates_mode >> 8;
      uint8_t RA2 = pinstates_mode >> 16;
      uint8_t RB2 = pinstates_mode >> 24;

      RA1 = ReverseUint8(RA1);
      //RB1 = ReverseUint8(RB1);
      RA2 = ReverseUint8(RA2);
      //RB2 = ReverseUint8(RB2);

      mcp.portMode(MCP23017Port::A, RA1, 0);
      mcp.portMode(MCP23017Port::B, RB1, 0);
      mcp2.portMode(MCP23017Port::A, RA2, 0);
      mcp2.portMode(MCP23017Port::B, RB2, 0);
    }

    uint8_t countPins(uint8_t type) {
      uint16_t looking = type;
      uint8_t c;
      for (byte i = 0; i < 32; i++) {
        if ((mapping[i] >> 8) == looking) {
          c++;
        }
      }
      return c;
    }
    bool IO16b = false;
    void preparePins() {
      mcp.portMode(MCP23017Port::A, MCP_INPUT);
      mcp.portMode(MCP23017Port::B, MCP_INPUT);
      mcp2.portMode(MCP23017Port::A, MCP_INPUT);
      mcp2.portMode(MCP23017Port::B, MCP_INPUT);
      this->pinModeByType(Def_GND, OUTPUT);
      this->pinModeByType(Def_WE, OUTPUT);
      this->pinModeByType(Def_CE, OUTPUT);
      this->pinModeByType(Def_OE, OUTPUT);
      this->pinModeByType(Def_A, OUTPUT);

      this->digitalWrite(this->getPin(Def_WE), HIGH);
      this->digitalWrite(this->getPin(Def_CE), HIGH);
      this->digitalWrite(this->getPin(Def_OE), HIGH);

      pinstates_read[0] = 0;
      pinstates_read[1] = 0;
      pinstates_read[2] = 0;
      pinstates_read[3] = 0;

      IO16b = false;
      if (countPins(Def_IO) > 8) {
        IO16b = true;
        debugln("16b word!");
      }
      debugln("8b word!");
    }
    void pinMode(uint8_t pin, bool mode) {
      pinstateWriteMode(pin, mode);
      applyModeState();
    }

    void pinModeByType(uint8_t type, uint8_t id, uint8_t mode) {
      for (byte i = 0; i < 32; i++) {
        if (mapping[i] == ((type << 8) | id)) {
          this->pinMode(i, mode);
        }
      }
    }
    void pinModeByType(uint8_t type, uint8_t mode) {
      for (byte i = 0; i < 32; i++) {
        if (mapping[i] >> 8 == type) {
          this->pinMode(i, mode);
        }
      }
    }

    void digitalWrite(uint8_t pin, bool state) {
      this->pinstateWrite(pin, state);
      this->applyState();
    }
    bool digitalRead(uint8_t pin) {
      if (pin >= 16) {
        return mcp2.digitalRead(pin % 16);
      } else {
        return mcp.digitalRead(pin % 16);
      }
    }

    bool digitalReadB(uint8_t pin) {
      if (pin >= 16) {
        pin = pin % 16;
        bool portB = false;
        if (pin > 7) {
          portB = true;
          pin -= 8;
        }
        return (bitRead(portB ? pinstates_read[3] : pinstates_read[2], pin));
      } else {
        pin = pin % 16;
        bool portB = false;
        if (pin > 7) {
          portB = true;
          pin -= 8;
        }
        return (bitRead(portB ? pinstates_read[1] : pinstates_read[0], pin));
      }
    }

    void writeAddress(uint16_t address) {
      for (byte i = 0; i < 16; i++) {
        uint8_t pin = getPin(Def_A, i);
        if (pin == 255)break;
        this->pinstateWrite(pin, (address >> i) & 1);
      }
      this->applyState();
    }

    void IODirection(uint8_t dir) {
      this->pinModeByType(Def_IO, dir);
    }

    void writeData(uint16_t data) {
      debugln();
      debug("WD");
      debug(data);
      for (byte i = 0; i < 16; i++) {
        uint8_t pin = getPin(Def_IO, i);
        if (pin == 255)break;
        bool state = (data >> i) & 1;
        this->pinstateWrite(pin, state);
        debug(pin);
        debug("=");
        debugln(state);
      }
      this->applyState();
    }
    uint16_t readData() {
      uint16_t data = 0;
      this->fetchRead();
      for (byte i = 0; i < 16; i++) {
        uint8_t pin = getPin(Def_IO, i);
        if (pin < 0)break;
        data |= (this->digitalReadB(pin) << i);
      }
      return data;
    }

    void fetchRead() {
      uint16_t r1 = mcp.read();
      uint16_t r2 = mcp2.read();
      pinstates_read[0] = r1;
      pinstates_read[1] = r1 >> 8;
      pinstates_read[2] = r2;
      pinstates_read[3] = r2 >> 8;
    }
};

const int cmd_count = 32;

typedef void (* VoidFunction)();

class Command {
  public:
    VoidFunction callback;
    String name;

    Command(VoidFunction callback, String name) {
      this->callback = callback;
      this->name = name;
    }

    Command() {

    }
};
static Command commands[cmd_count];
static int registerIndex = 0;
ESP8266WebServer server(80);


void RegisterCommand(void (* callback)(), String name) {
  if (registerIndex >= cmd_count) {
    return;
  }
  commands[registerIndex] = Command(callback, name);
  registerIndex++;
  server.on(name, callback);
}

EEPROMDef eeprom;

String data;

void setup(void) {
  WiFi.persistent(false);
  if (!LittleFS.begin()) {
    Serial.println("An Error has occurred while mounting LittleFS");
  }

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(115200);
  Serial.println("");
  Wire.setClock(I2C_SPEED);
  Wire.begin();

  Serial.print(F("System information:"));
  Serial.print(F("\nI2C_CLOCK: "));
  Serial.println(I2C_SPEED);

  WifiReadConfig();
  WifiConnect();

  if (MDNS.begin("esp8266")) {
    Serial.println(F("MDNS responder started"));
  }

  server.on("/", handleRoot);
  RegisterCommands();
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println(F("HTTP server started"));

  mcp.init();
  mcp2.init();

  CheckMCPConnection();

  digitalWrite(LED_BUILTIN, LOW);
  eeprom.SetupDefaultMappings();
  Serial.setTimeout(1000);
  MCPBenchMark();
  MCPReset();
}

void loop(void) {
  IO_Mode(IO_HTTP);
  server.handleClient();

  if (Serial.available()) {
    uint8_t read = Serial.read();

    if (read == 'r') {
      Serial.println("Write: 'y' to confirm wifi reset! You have 5s");
      long start = millis();
      while ((millis() - start) < 5000) {
        if (Serial.available())break;
      }
      read = Serial.read();
      if (read == 'y') {
        Serial.println("Resetting...");
        WifiResetConfig();
        Serial.println("Reset completed!");
        WifiConnect();
      } else {
        Serial.println("Reset aborted!");
      }
      Serial.write('\0');
    }
    if (read == 'c') {
      IO_Mode(IO_SERIAL);
      String read = Serial.readStringUntil('\n');
      ExecuteCmd(read);
      Serial.write('\0');
      return;
    }
    if (read == 'w') {
      String confJSON = "";
      AppendJSON(confJSON, wifi_json);
      Serial.println(confJSON);
      Serial.write('\0');
      return;
    }
    if (read == 'd') {
      uint8_t bytes[1];
      Serial.readBytes(bytes, 1);
      debug_ovr = bytes[0] == 1;
      Serial.println(debug_ovr);
      Serial.write('\0');
      return;
    }
    if (read == 'i') {
      uint8_t bytes[1];
      Serial.readBytes(bytes, 1);
      if (bytes[0] == 'p') {
        SendIpData();
        return;
      }
    }
  }
}
