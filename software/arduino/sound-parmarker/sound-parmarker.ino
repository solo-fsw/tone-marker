/*
  Usb Parallel Marker Versie
  Copyright Leiden University SOLO - Evert 2018~2023
*/
// #define stQ 11  // PB3 - PCINT3
// #define Q1_pin 38  // PF5 - ADC5
// #define Q2_pin 37  // PF6 - ADC6
// #define Q3_pin 36  // PF7 - ADC7
// #define Q4_pin 40  // PF1 - ADC1

#undef TX_RX_LED_INIT
#undef TXLED1
#undef RXLED0
#undef RXLED1
#define LEDCC 7
#include <EEPROM.h>

//Globals
const String SwVer = "SW1.7";
const String Version; 
const String Serialno;
const String HwVer;

//Soundmarker globals
char buffer [200] = {0};
unsigned int bufferLength = 0;
bool on = false;

void receive() {
  bool Q1 = ((PINF >> 5)  & 0x01);
  bool Q2 = ((PINF >> 6)  & 0x01);
  bool Q3 = ((PINF >> 7)  & 0x01);
  bool Q4 = ((PINF >> 1)  & 0x01);
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
  writeStringToEEPROM(10, "S01116");    //Use ones to program te serial number in the eeprom of the device
  writeStringToEEPROM(20, "HW1");    //Use ones to program the hardware version in the eeprom of the device
  DDRD = 0xFF; //PortD all pins output
  PORTD = 0x00;
  DDRF = DDRF | B00011101;  //PortF pins 7, 6, 5 and 1 Input
  PORTF = 0x00;
  PCICR = PCICR | B00000001;
  PCMSK0 = PCMSK0 | B00001000;
  pinMode(LEDCC, OUTPUT);
  digitalWrite(LEDCC, LOW);
  Serial.begin(115200);     // opens serial port, sets data rate to 115200 bps
  Serialno = readStringFromEEPROM(10);
  HwVer= readStringFromEEPROM(20);
  Version = String(HwVer + ":" + SwVer);   // Set HW version always 
  TXLED0;
}

ISR(PCINT0_vect) {
  cli(); // Disable interrupts
  if(!on){
    receive();
  }
  on = !on;
  sei();  // Enable interrupts
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
    int markerValue = atoi(substr);
    if (markerValue < 0){
      // TODO: Overflow error!
    } else if (markerValue <= 255){
      Serial.print("Marker: ");
      Serial.println(markerValue);
      PORTD = markerValue;
      delay(10);
      PORTD = 0x00;
    } else if (markerValue >= 256){
      Serial.print("Remain at ");
      Serial.print(markerValue % 255);
      Serial.print(" instead of ");
      Serial.println(markerValue);
      PORTD = markerValue % 255;
    }
  }
}