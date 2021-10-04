byte i_position = 0;
String str_buff = "";

#define BlockSize 32

void appendSegment(char c) {
  str_buff[i_position] = c;
  if ((i_position % BlockSize) == BlockSize-1) {
    
    flushSegment();
  }
  i_position ++;
}

void beginSegment() {
  i_position = 0;
}

void flushSegment() {
  if (i_position > 0) {
    char buff[BlockSize];
    str_buff.toCharArray(buff, BlockSize);
    server.sendContent(buff, i_position);
    i_position = -1;
  }
}

//server.sendContent((String)((char)b));
