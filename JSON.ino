bool ReadJSON(String& input, DynamicJsonDocument& json) {
  DeserializationError error = deserializeJson(json, input);
  if (error) {
    Serial.print(F("readJson() failed: "));
    Serial.println(error.f_str());
    return false;
  }
  return true;
}

bool ReadJSON(String& input) {
  return ReadJSON(input, json);
}

bool ReadJSONSV() {
  return ReadJSONSV(json);
}

bool ReadJSONSV(DynamicJsonDocument& json) {
  String read = String(server.arg("plain"));
  return ReadJSON(read, json);
}

void AppendJSON(String& output) {
  AppendJSON(output, json);
}

void AppendJSON(String& output, DynamicJsonDocument& json) {
  serializeJson(json, output);
}

void ClearJSON() {
  json.clear();
}

void ClearJSON(DynamicJsonDocument& json) {
  json.clear();
}

bool TransferProperty(DynamicJsonDocument& destination, DynamicJsonDocument& source, char* property, bool skipNull = false) {
  JsonVariant jv = source[property];
  if (jv.isNull() && skipNull) {
    return false;
  }
  destination[property] = jv;
  return true;
}

bool TransferPropertyMsg(DynamicJsonDocument& destination, DynamicJsonDocument& source, char* property, char* name = "", bool skipNull = false) {
  debug(name);
  debug(": Property: ");
  debug(property);

  bool res = TransferProperty(destination, source, property, skipNull);

  debug(res ? " - Changed to" : " Skipped");
  if (res) debug(destination[property].as<String>());
  debugln("");
  return res;
}


bool TransferPropertyMsg(DynamicJsonDocument& destination, DynamicJsonDocument& source, char* property, bool skipNull = false) {
  return TransferPropertyMsg(destination, source, property, "", skipNull);
}
