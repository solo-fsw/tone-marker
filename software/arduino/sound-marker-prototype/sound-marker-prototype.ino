#define stQ 2
#define Q1_pin 8
#define Q2_pin 9
#define Q3_pin 10
#define Q4_pin 11

char buffer [200] = {0};
unsigned int bufferLength = 0;

void receive() {
  bool Q1 = digitalRead(Q1_pin);
  bool Q2 = digitalRead(Q2_pin);
  bool Q3 = digitalRead(Q3_pin);
  bool Q4 = digitalRead(Q4_pin);

  byte data = (0x00 | Q1 | (Q2 << 1) | (Q3 << 2) | (Q4 << 3));
  char symbol = decode_signal(data);
  if(bufferLength < 200){
    buffer[bufferLength++] = symbol;
  } else {
    // TODO: Error!
  }
}

char decode_signal(byte signal) {
  switch (signal) {
    case 1:  return '1';
    case 2:  return '2';
    case 3:  return '3';
    case 4:  return '4';
    case 5:  return '5';
    case 6:  return '6';
    case 7:  return '7';
    case 8:  return '8';
    case 9:  return '9';
    case 10: return '0';
    case 11: return '*';
    case 12: return '#';
    case 13: return 'A';
    case 14: return 'B';
    case 15: return 'C';
    case 0:  return 'D';
    default: return '?';
  }
}

void setup() {
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(stQ), receive, RISING);
}

void loop() {
  char substr[200] = {0};
  for (int i = 0 ; i < bufferLength ; i++){
    if (buffer[i] == '*'){
      for (int j = i; j < bufferLength ; j++){
        if (buffer[j] == '#'){
            strncpy(substr, buffer + i + 1, j - i - 1);
            substr[j - i - 1] = 0;
            buffer[200] = {0};
            bufferLength = 0;
          break;
        }
      }
      break;
    }
  }
  if (substr[0] != '\0'){
    Serial.println(substr);
  }
}
