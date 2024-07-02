#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <FastLED.h>

// https://forum.pjrc.com/index.php?threads/available-teensy-4-0-pins-when-audio-shield-d-attached.58331/
// https://github.com/luni64/TeensyTimerTool/wiki/Avoid-PWM-timer-clashes
// https://forum.pjrc.com/index.php?threads/teensy-4-1-digital-i-o-pin-map.64226/
// https://forum.pjrc.com/index.php?threads/tutorial-on-digital-i-o-atmega-pin-port-ddr-d-b-registers-vs-arm-gpio_pdir-_pdor.17532/
// https://forum.pjrc.com/index.php?threads/speed-of-digitalread-and-digitalwrite-with-teensy3-0.24573/
// https://forum.pjrc.com/index.php?threads/unclear-on-how-to-use-ddrx-and-portx-teensy-3-2.53950/
// https://arduino.stackexchange.com/questions/72440/what-is-the-equivilent-of-portx-for-teensy-4-0

#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  11
#define SDCARD_SCK_PIN   13
#define DATA_PIN 14

#define N_LEDS 8
#define STRIPS 1

// Register?
struct BitState {
  bool state;
  int bitIdx;
  int pinNr;
  int registerIdx;
};

struct BitState bit0, bit1, bit2, bit3, bit4, bit5, bit6, bit7;

CRGB leds[N_LEDS * STRIPS];

elapsedMillis msecs;
const float THRESHOLD = 0.95;  // Maximum value is correlated with the volume percentage of the connected pc
const float FILTER_FREQ = 14000.0;
const float DETECTION_THRESHOLD = 0.90;

byte markerRegister = 0x00;

// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=285,359
AudioFilterBiquad        filterMarker;   //xy=526,453
AudioMixer4              mixerRight;     //xy=548,369
AudioMixer4              mixerLeft;      //xy=551,280
AudioFilterBiquad        filterRight;    //xy=893,357
AudioFilterBiquad        filterLeft;     //xy=895,296
AudioMixer4              mixerMarker;    //xy=961,464
AudioAnalyzePeak         peakMarker;     //xy=1143,464
AudioOutputI2S           i2s2;           //xy=1157,326
AudioAnalyzeToneDetect   toneMarker3; //xy=1199,664
AudioAnalyzeToneDetect   toneMarker4; //xy=1203,712
AudioAnalyzeToneDetect   toneMarker2;    //xy=1204,616
AudioAnalyzeToneDetect   toneMarker1;    //xy=1205,570
AudioConnection          patchCord1(i2s1, 0, mixerLeft, 0);
AudioConnection          patchCord2(i2s1, 1, filterMarker, 0);
AudioConnection          patchCord3(i2s1, 1, mixerRight, 0);
AudioConnection          patchCord4(filterMarker, 0, mixerMarker, 0);
AudioConnection          patchCord5(mixerRight, filterRight);
AudioConnection          patchCord6(mixerLeft, filterLeft);
AudioConnection          patchCord7(filterRight, 0, i2s2, 1);
AudioConnection          patchCord8(filterLeft, 0, i2s2, 0);
AudioConnection          patchCord9(mixerMarker, peakMarker);
AudioConnection          patchCord10(mixerMarker, toneMarker1);
AudioConnection          patchCord11(mixerMarker, toneMarker2);
AudioConnection          patchCord12(mixerMarker, toneMarker3);
AudioConnection          patchCord13(mixerMarker, toneMarker4);
AudioControlSGTL5000     sgtl5000_1;     //xy=827,182
// GUItool: end automatically generated code


void setup(){
  Serial.begin(9600);

  AudioMemory(20);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);

  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);

  filterLeft.setLowpass(0, FILTER_FREQ, 1.0);
  filterRight.setLowpass(0, FILTER_FREQ, 1.0);

  filterLeft.setLowpass(1, FILTER_FREQ, 1.0);
  filterRight.setLowpass(1, FILTER_FREQ, 1.0);

  filterMarker.setHighpass(0, FILTER_FREQ, 10); // Leave at Q = 10

  mixerLeft.gain(0, 1.0);  // lowpass
  mixerRight.gain(0, 1.0);  // lowpass
  mixerMarker.gain(0, 1.5);  // highpass

  toneMarker1.frequency(16000, 40);  // Keep a distance of approximately 200hz around either side of a marker tone
  toneMarker2.frequency(16500, 40);
  toneMarker3.frequency(17000, 40);
  toneMarker4.frequency(17500, 40);

  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);

  delay(1000);

  FastLED.addLeds<STRIPS, WS2812, DATA_PIN, GRB>(leds, N_LEDS);
  FastLED.setBrightness(100);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 1500);
  fill_solid(leds, N_LEDS, CRGB::Black);
  FastLED.show();

  bit0.state = 0;
}


void loop(){
  if(msecs > 50){
    if(peakMarker.available()){
      msecs = 0;
      float peakValue = peakMarker.read();
      if(peakValue > THRESHOLD){
        bool m1 = 0;
        bool m2 = 0;
        bool m3 = 0;
        bool m4 = 0;
        if(toneMarker1.read() > DETECTION_THRESHOLD){
          m1 = 1;
          Serial.println("Bit 0 (pin 10)");
        } 
        if(toneMarker2.read() > DETECTION_THRESHOLD){
          m2 = 1;
          Serial.println("Bit 1 (pin 12)");
        }
        if(toneMarker3.read() > DETECTION_THRESHOLD){
          m3 = 1;
          Serial.println("Bit 2 (pin 11)");
        }
        if(toneMarker4.read() > DETECTION_THRESHOLD){
          m4 = 1;
          Serial.println("Bit 3 (pin 13)");
        }
        markerRegister = (0x00 | m1 | (m2 << 1) | (m3 << 2) | (m4 << 3));
        GPIO7_DR &= 0b0000;
        GPIO7_DR |= markerRegister;

        fill_solid( leds, N_LEDS, CRGB::Red);
        FastLED.show();
      }
    } else{
          fill_solid( leds, N_LEDS, CRGB::Black);
    FastLED.show();
    }
  }
}
