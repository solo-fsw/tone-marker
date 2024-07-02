#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <FastLED.h>

#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  11
#define SDCARD_SCK_PIN   13
#define DATA_PIN 11
#define N_LEDS 8
#define STRIPS 1

// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=285,359
AudioFilterBiquad        filterMarker;   //xy=526,453
AudioMixer4              mixerRight;     //xy=548,369
AudioMixer4              mixerLeft;      //xy=551,280
AudioMixer4              mixerMarker;    //xy=691,472
AudioFilterBiquad        filterRight;    //xy=893,357
AudioFilterBiquad        filterLeft;     //xy=895,296
AudioAnalyzePeak         peakMarker;     //xy=1143,464
AudioOutputI2S           i2s2;           //xy=1157,326
AudioAnalyzeToneDetect   toneMarker3; //xy=1199,664
AudioAnalyzeToneDetect   toneMarker4; //xy=1203,712
AudioAnalyzeToneDetect   toneMarker2;    //xy=1204,616
AudioAnalyzeToneDetect   toneMarker1;    //xy=1205,570
AudioAnalyzeToneDetect   toneMarker5; //xy=1210,765
AudioAnalyzeToneDetect   toneMarker8; //xy=1210,909
AudioAnalyzeToneDetect   toneMarker6; //xy=1215,811
AudioAnalyzeToneDetect   toneMarker7; //xy=1218,856
AudioConnection          patchCord1(i2s1, 0, mixerLeft, 0);
AudioConnection          patchCord2(i2s1, 1, filterMarker, 0);
AudioConnection          patchCord3(i2s1, 1, mixerRight, 0);
AudioConnection          patchCord4(filterMarker, 0, mixerMarker, 0);
AudioConnection          patchCord5(mixerRight, filterRight);
AudioConnection          patchCord6(mixerLeft, filterLeft);
AudioConnection          patchCord7(mixerMarker, peakMarker);
AudioConnection          patchCord8(mixerMarker, toneMarker1);
AudioConnection          patchCord9(mixerMarker, toneMarker2);
AudioConnection          patchCord10(mixerMarker, toneMarker3);
AudioConnection          patchCord11(mixerMarker, toneMarker4);
AudioConnection          patchCord12(mixerMarker, toneMarker5);
AudioConnection          patchCord13(mixerMarker, toneMarker6);
AudioConnection          patchCord14(mixerMarker, toneMarker7);
AudioConnection          patchCord15(mixerMarker, toneMarker8);
AudioConnection          patchCord16(filterRight, 0, i2s2, 1);
AudioConnection          patchCord17(filterLeft, 0, i2s2, 0);
AudioControlSGTL5000     sgtl5000_1;     //xy=827,182
// GUItool: end automatically generated code

// Register GPIO6
struct BitState {
  bool state;
  int bitIdx;
  int pinNr;
  int registerIdx;
  float frequency;
  AudioAnalyzeToneDetect *freq_filter;
};

// Initialize bits
BitState bits[8] = {
  {0, 0, 1, 2, 16000, &toneMarker1},
  {0, 1, 0, 3, 16500, &toneMarker2},
  {0, 2, 24, 12, 17000, &toneMarker3},
  {0, 3, 26, 16, 17500, &toneMarker4},
  {0, 4, 14, 18, 20000, &toneMarker5},
  {0, 5, 17, 22, 20000, &toneMarker6},
  {0, 6, 16, 23, 20000, &toneMarker7},
  {0, 7, 22, 24, 20000, &toneMarker8}
};
uint32_t markerRegister = 0;

// Declare led strip
CRGB leds[N_LEDS * STRIPS];

// Declare global variables and constants
elapsedMillis msecs;
const float THRESHOLD = 0.95;  // Maximum value is correlated with the volume percentage of the connected pc
const float FILTER_FREQ = 14000.0;
const float DETECTION_THRESHOLD = 0.90;


void setup(){
  // Start serial connection for debugging
  Serial.begin(9600);

  // Initialize audio buffer and start main audio controller
  AudioMemory(20);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);

  // Set SD card variables
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);

  // Initialize lowpass filters
  filterLeft.setLowpass(0, FILTER_FREQ, 1.0);
  filterRight.setLowpass(0, FILTER_FREQ, 1.0);

  filterLeft.setLowpass(1, FILTER_FREQ, 1.0);
  filterRight.setLowpass(1, FILTER_FREQ, 1.0);

  // Initialize highpass filter
  filterMarker.setHighpass(0, FILTER_FREQ, 10); // Leave at Q = 10

  // Set gains
  mixerLeft.gain(0, 1.0);  // lowpass
  mixerRight.gain(0, 1.0);  // lowpass
  mixerMarker.gain(0, 2.0);  // highpass

  // Initialize led strip
  FastLED.addLeds<STRIPS, WS2812, DATA_PIN, GRB>(leds, N_LEDS);
  FastLED.setBrightness(40);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 1500);

  // Initialize marker pins and corresponding frequency filters
  for(int idx = 0; idx < 8; idx++){
    pinMode(bits[idx].pinNr, OUTPUT);
    bits[idx].freq_filter->frequency(bits[idx].frequency, 40);
  }
  // toneMarker1.frequency(bits[0].frequency, 40);
}

void loop(){
  if(msecs > 50){
    if(peakMarker.available() && peakMarker.read() > THRESHOLD){
      msecs = 0;
      for(int idx = 0; idx < 8; idx++){
        bits[idx].state = bits[idx].freq_filter->read() > DETECTION_THRESHOLD;
        GPIO6_DR = (GPIO6_DR & ~(1 << bits[idx].registerIdx)) | (bits[idx].state << bits[idx].registerIdx);
        Serial.print(idx);
        Serial.print("  ");
        // Serial.println(bits[idx].freq_filter->read());
        Serial.println(bits[idx].state);
      }
    }
  }
  for(int idx = 0; idx < 8; idx++){
    if(bits[idx].state){
      leds[idx] = CRGB::HotPink;
    } else {
      leds[idx] = CRGB::Black;
    }
  }
  FastLED.show();
}

        // markerRegister = (0x00 | m1 | (m2 << 1) | (m3 << 2) | (m4 << 3));
        // GPIO6_DR &= 0b0000;
        // GPIO6_DR |= markerRegister;

        // fill_solid( leds, N_LEDS, CRGB::Red);
        // FastLED.show();

/*
Usefull links:
https://forum.pjrc.com/index.php?threads/available-teensy-4-0-pins-when-audio-shield-d-attached.58331/
https://github.com/luni64/TeensyTimerTool/wiki/Avoid-PWM-timer-clashes
https://forum.pjrc.com/index.php?threads/teensy-4-1-digital-i-o-pin-map.64226/
https://forum.pjrc.com/index.php?threads/tutorial-on-digital-i-o-atmega-pin-port-ddr-d-b-registers-vs-arm-gpio_pdir-_pdor.17532/
https://forum.pjrc.com/index.php?threads/speed-of-digitalread-and-digitalwrite-with-teensy3-0.24573/
https://forum.pjrc.com/index.php?threads/unclear-on-how-to-use-ddrx-and-portx-teensy-3-2.53950/
https://arduino.stackexchange.com/questions/72440/what-is-the-equivilent-of-portx-for-teensy-4-0
*/
