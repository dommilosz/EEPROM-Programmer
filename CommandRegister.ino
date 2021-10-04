void ExecuteCmd(String name) {
  int cmd = GetCommand(name);
  if (cmd > 0) {
    commands[cmd].callback();
  }

}

int GetCommand(String name) {
  for (int i = 0; i < cmd_count; i++) {
    if (commands[i].name == name) {
      return i;
    }
  }
  return -1;
}

void RegisterCommands() {
  RegisterCommand(handleRead, "/read/");
  RegisterCommand(handleWrite, "/write/");
  RegisterCommand(handleMCPDWrite, "/mcpdwrite/");
  RegisterCommand(handleMCPWrite, "/mcpwrite/");
  RegisterCommand(handleMcpDump, "/mcpdump/");
  RegisterCommand(handleJSON, "/json/");
  RegisterCommand(handlePinDefConfiguration, "/pinconf/");
  RegisterCommand(handleAIOWrite, "/mcpIOTest/");
  RegisterCommand(handleWifiConfig, "/wificonf/");
}
