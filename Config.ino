void SendMappings() {
  ClearJSON();
  JsonArray array = json.createNestedArray("mapping");

  for (int i = 0; i < 32; i++) {
    array[i] = eeprom.mapping[i];
  }

  String msg;
  AppendJSON(msg);
  IO_SendHead(IO_BOTH, msg);
}

void LoadMappings() {
  IO_ReadJSON(IO_BOTH,json);
  JsonArray array = json["mapping"];

  for (int i = 0; i < 32; i++) {
    eeprom.mapping[i] = array[i];
  }
  eeprom.CompileMappings();
}
