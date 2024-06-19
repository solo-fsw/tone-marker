/*
  Usb Parallel Marker Versie
  Copyright Leiden University SOLO - Evert 2018~2023
*/
// stQ 11  // PB3 - PCINT3
// Q1_pin 38  // PF5 - ADC5
// Q2_pin 37  // PF6 - ADC6
// Q3_pin 36  // PF7 - ADC7
// Q4_pin 40  // PF1 - ADC1

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

void setup() {
  writeStringToEEPROM(10, "S01116");    //Use ones to program te serial number in the eeprom of the device
  writeStringToEEPROM(20, "HW1");    //Use ones to program the hardware version in the eeprom of the device
  DDRD = 0xFF; //PortD all pins output
  PORTD = 0x00;
  pinMode(LEDCC, OUTPUT);
  digitalWrite(LEDCC, LOW);
  Serial.begin(115200);     // opens serial port, sets data rate to 115200 bps
  Serialno = readStringFromEEPROM(10);
  HwVer= readStringFromEEPROM(20);
  Version = String(HwVer + ":" + SwVer);   // Set HW version always 
  TXLED0;

  DDRF = DDRF | B00011101;  //PortF pins 7, 6, 5 and 1 Input
  PORTF = 0x00;
  PCICR = PCICR | B00000001;
  PCMSK0 = PCMSK0 | B00001000;

  PORTD = 0xFF;
  delay(1000);
  PORTD = 0x00;
}

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
  char conversion[16] = {'D', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '*', '#', 'A', 'B', 'C'};
  return conversion[signal];
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
      Serial.print("Marker set to: ");
      Serial.println(markerValue);
      PORTD = markerValue;
    } else if (markerValue >= 256){
      Serial.print("Remain at ");
      Serial.print(markerValue % 255);
      Serial.print(" for 100 ms instead of ");
      Serial.println(markerValue);
      PORTD = markerValue % 255;
      delay(100);
      PORTD = 0x00;
    }
  }
}