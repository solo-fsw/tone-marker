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
AudioInputI2S            i2s1;           //xy=101,321
AudioFilterBiquad        filterMarker;   //xy=346,417
AudioMixer4              mixerLeft;      //xy=348,268
AudioMixer4              mixerRight;     //xy=348,348
AudioMixer4              mixerMarker;    //xy=572,436
AudioFilterBiquad        filterRight;    //xy=580,347
AudioFilterBiquad        filterLeft;     //xy=584,268
AudioOutputI2S           i2s2;           //xy=781,307
AudioAnalyzePeak         peakMarker;     //xy=945,438
AudioFilterBiquad        filterBand1;        //xy=946,493
AudioFilterBiquad        filterBand2; //xy=949,544
AudioFilterBiquad        filterBand3; //xy=949,595
AudioFilterBiquad        filterBand4; //xy=950,649
AudioFilterBiquad        filterBand5; //xy=952,698
AudioFilterBiquad        filterBand6; //xy=955,747
AudioFilterBiquad        filterBand7; //xy=957,795
AudioFilterBiquad        filterBand8; //xy=957,841
AudioMixer4              mixerMarker1;         //xy=1261,409
AudioMixer4              mixerMarker2; //xy=1265,497
AudioMixer4              mixerMarker3; //xy=1273,584
AudioMixer4              mixerMarker4; //xy=1274,664
AudioMixer4              mixerMarker5; //xy=1274,744
AudioMixer4              mixerMarker6; //xy=1279,828
AudioMixer4              mixerMarker7; //xy=1280,913
AudioMixer4              mixerMarker8; //xy=1283,988
AudioAnalyzeToneDetect   toneMarker1;    //xy=1506,410
AudioAnalyzeToneDetect   toneMarker2;    //xy=1510,498
AudioAnalyzeToneDetect   toneMarker3; //xy=1512,584
AudioAnalyzeToneDetect   toneMarker4; //xy=1512,664
AudioAnalyzeToneDetect   toneMarker5; //xy=1514,742
AudioAnalyzeToneDetect   toneMarker7; //xy=1516,913
AudioAnalyzeToneDetect   toneMarker8; //xy=1518,991
AudioAnalyzeToneDetect   toneMarker6; //xy=1519,826
AudioConnection          patchCord1(i2s1, 0, mixerLeft, 0);
AudioConnection          patchCord2(i2s1, 1, filterMarker, 0);
AudioConnection          patchCord3(i2s1, 1, mixerRight, 0);
AudioConnection          patchCord4(filterMarker, 0, mixerMarker, 0);
AudioConnection          patchCord5(mixerLeft, filterLeft);
AudioConnection          patchCord6(mixerRight, filterRight);
AudioConnection          patchCord7(mixerMarker, peakMarker);
AudioConnection          patchCord8(mixerMarker, filterBand1);
AudioConnection          patchCord9(mixerMarker, filterBand2);
AudioConnection          patchCord10(mixerMarker, filterBand3);
AudioConnection          patchCord11(mixerMarker, filterBand4);
AudioConnection          patchCord12(mixerMarker, filterBand5);
AudioConnection          patchCord13(mixerMarker, filterBand6);
AudioConnection          patchCord14(mixerMarker, filterBand7);
AudioConnection          patchCord15(mixerMarker, filterBand8);
AudioConnection          patchCord16(filterRight, 0, i2s2, 1);
AudioConnection          patchCord17(filterLeft, 0, i2s2, 0);
AudioConnection          patchCord18(filterBand1, 0, mixerMarker1, 0);
AudioConnection          patchCord19(filterBand2, 0, mixerMarker2, 0);
AudioConnection          patchCord20(filterBand3, 0, mixerMarker3, 0);
AudioConnection          patchCord21(filterBand4, 0, mixerMarker4, 0);
AudioConnection          patchCord22(filterBand5, 0, mixerMarker5, 0);
AudioConnection          patchCord23(filterBand6, 0, mixerMarker6, 0);
AudioConnection          patchCord24(filterBand7, 0, mixerMarker7, 0);
AudioConnection          patchCord25(filterBand8, 0, mixerMarker8, 0);
AudioConnection          patchCord26(mixerMarker1, toneMarker1);
AudioConnection          patchCord27(mixerMarker2, toneMarker2);
AudioConnection          patchCord28(mixerMarker3, toneMarker3);
AudioConnection          patchCord29(mixerMarker4, toneMarker4);
AudioConnection          patchCord30(mixerMarker5, toneMarker5);
AudioConnection          patchCord31(mixerMarker6, toneMarker6);
AudioConnection          patchCord32(mixerMarker7, toneMarker7);
AudioConnection          patchCord33(mixerMarker8, toneMarker8);
AudioControlSGTL5000     sgtl5000_1;     //xy=827,182
// GUItool: end automatically generated code

// Register GPIO6
struct BitState {
  bool state;
  int bitIdx;
  int pinNr;
  int registerIdx;
  float frequency;
  float gain;
  AudioFilterBiquad *band_filter;
  AudioMixer4 *amplifier;
  AudioAnalyzeToneDetect *freq_filter;
};

// Initialize bits
BitState bits[8] = {
  {0, 0, 1, 2, 16000, 1.0, &filterBand1, &mixerMarker1, &toneMarker1},
  {0, 1, 0, 3, 16500, 1.0, &filterBand2, &mixerMarker2, &toneMarker2},
  {0, 2, 24, 12, 17000, 1.0, &filterBand3, &mixerMarker3, &toneMarker3},
  {0, 3, 26, 16, 17500, 1.2, &filterBand4, &mixerMarker4, &toneMarker4},
  {0, 4, 14, 18, 20000, 1.0, &filterBand5, &mixerMarker5, &toneMarker5},
  {0, 5, 17, 22, 20000, 1.0, &filterBand6, &mixerMarker6, &toneMarker6},
  {0, 6, 16, 23, 20000, 1.0, &filterBand7, &mixerMarker7, &toneMarker7},
  {0, 7, 22, 24, 20000, 1.0, &filterBand8, &mixerMarker8, &toneMarker8}
};

// Declare led strip
CRGB leds[N_LEDS * STRIPS];

// Declare global variables and constants
elapsedMillis msecs;
const float AMPLITUDE_THRESHOLD = 0.95;  // Maximum value is correlated with the volume percentage of the connected pc
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
  mixerLeft.gain(0, 0.5);  // lowpass
  mixerRight.gain(0, 0.5);  // lowpass
  mixerMarker.gain(0, 2.0);  // highpass

  // Initialize led strip
  FastLED.addLeds<STRIPS, WS2812, DATA_PIN, GRB>(leds, N_LEDS);
  FastLED.setBrightness(40);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 1500);

  // Initialize marker pins and corresponding frequency filters
  for(int idx = 0; idx < 8; idx++){
    pinMode(bits[idx].pinNr, OUTPUT);
    bits[idx].band_filter->setBandpass(0, bits[idx].frequency, 10.0);
    bits[idx].amplifier->gain(0, bits[idx].gain);
    bits[idx].freq_filter->frequency(bits[idx].frequency, 80);
  }
}

void loop(){
  if(msecs > 50){
    if(peakMarker.available() && peakMarker.read() > AMPLITUDE_THRESHOLD){
      msecs = 0;
      for(int idx = 0; idx < 8; idx++){
        bits[idx].state = bits[idx].freq_filter->read() > DETECTION_THRESHOLD;
        GPIO6_DR = (GPIO6_DR & ~(1 << bits[idx].registerIdx)) | (bits[idx].state << bits[idx].registerIdx);
        // Serial.print(idx);
        // Serial.print("  ");
        // Serial.println(bits[idx].freq_filter->read());
      }
    }
  }
  if(msecs > 200){
    msecs = 0;
    for(int idx = 0; idx < 8; idx++){
      bits[idx].state = 0;
      GPIO6_DR = (GPIO6_DR & ~(1 << bits[idx].registerIdx)) | (bits[idx].state << bits[idx].registerIdx);
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
