void writeStringToEEPROM(int addrOffset, const String &strToWrite)
{
  byte len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++)
  {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
}

String readStringFromEEPROM(int addrOffset)
{
  int newStrLen = EEPROM.read(addrOffset);
  char data[newStrLen + 1];
  for (int i = 0; i < newStrLen; i++)
  {
    data[i] = EEPROM.read(addrOffset + 1 + i);
  }
  data[newStrLen] = '\0'; // the character may appear in a weird way, you should read: 'only one backslash and 0'
  return String(data);
}

void DumpMyInfo() {
  Serial.print("{\"Version\":\"");
  Serial.print(Version);
  Serial.print("\",");
  Serial.print("\"Serialno\":\"");
  Serial.print(Serialno);
  Serial.print("\",");
  Serial.println("\"Device\":\"UsbParMarker\"}");
  //Serial.write(ETX);
}

void handlecommands() {
  switch (Serial.read()) {
    case 'V':
      DumpMyInfo();
      break;
    case 'P':
      Serial.println("Pong,UsbParMarker");
      break;
    //#ifdef HwVer "HW3"
    case 'L':
      digitalWrite(LEDCC, LOW);
      Serial.println("LedsEnabled");
      break;
    case 'O':
      digitalWrite(LEDCC, HIGH);
      Serial.println("LedsDisabled");
      break;
    case 'F':
      digitalWrite(LEDCC, LOW);
      PORTD = 0x00;
      Serial.println("LedTest");
      for (int i = 1; i <= 2; i++) {
        PORTD = 0xFF;
        delay(1000);
        PORTD = 0x00;
        delay(1000);
      }
      break;
    //#endif
    default:
      Serial.println("Unknown command");
      break;
  }
}
