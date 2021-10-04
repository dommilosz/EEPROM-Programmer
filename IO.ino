byte CURR_TYPE = IO_HTTP;
HTTPMethod SerialMethod = HTTP_GET;

void IO_Mode(byte type){
  if(type==IO_HTTP||type==IO_SERIAL){
    CURR_TYPE = type;
  }
  if(type==IO_SERIAL){
    String read = Serial.readStringUntil('\n');
    if(read=="GET"){
      SerialMethod = HTTP_GET;
    }
    if(read=="POST"){
      SerialMethod = HTTP_POST;
    }
  }
}

HTTPMethod Method(){
  if(CURR_TYPE == IO_HTTP){
    return server.method();
  }
  return SerialMethod;
}
 
bool IO_ReadJSON(byte type, DynamicJsonDocument& json){
  if(!IO_CheckType(type))return false;

  if(CURR_TYPE==IO_HTTP){
    return ReadJSONSV(json);
  }
  if(CURR_TYPE==IO_SERIAL){
    return ReadJSONSerial(json);
  }
  return false;
}

void IO_Send(byte type, String msg){
  if(!IO_CheckType(type))return;

  if(CURR_TYPE==IO_HTTP){
    server.sendContent(msg);
  }
  if(CURR_TYPE==IO_SERIAL){
    Serial.print(msg);
  }
}

void IO_SendHead(byte type, String msg){
  if(!IO_CheckType(type))return;

  if(CURR_TYPE==IO_HTTP){
    server.send(200, "text/plain", msg);
  }
  if(CURR_TYPE==IO_SERIAL){
    Serial.print(msg);
  }
}

bool IO_CheckType(byte type){
  if(type==IO_BOTH || CURR_TYPE == type){
    return true; 
  }
  return false;
}

bool IO_Is(byte type){
  if(CURR_TYPE == type){
    return true; 
  }
  return false;
}
