void SendMappings() {
  ClearJSON();
  JsonArray array = json.createNestedArray("mapping");

  for (int i = 0; i < 32; i++) {
    array[i] = eeprom.mapping[i];
  }

  String msg;
  AppendJSON(msg);
  server.send(200, "text/plain", msg);
}

void LoadMappings() {
  ReadJSONSV();
  JsonArray array = json["mapping"];

  for (int i = 0; i < 32; i++) {
    eeprom.mapping[i] = array[i];
  }
  eeprom.CompileMappings();
}
