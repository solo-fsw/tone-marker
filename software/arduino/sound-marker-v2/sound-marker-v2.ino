#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <FastLED.h>

#define DATA_PIN 17
#define N_LEDS 1
#define STRIPS 1
#define N_BITS 3
#define N_UNUSED_BITS 5

// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=101,321
AudioFilterBiquad        filterMarker;   //xy=346,417
AudioMixer4              mixerLeft;      //xy=348,268
AudioMixer4              mixerRight;     //xy=348,348
AudioMixer4              mixerMarker;    //xy=572,436
AudioFilterBiquad        filterRight;    //xy=580,347
AudioFilterBiquad        filterLeft;     //xy=584,268
AudioOutputI2S           i2s2;           //xy=781,307
AudioFilterBiquad        filterBand1;        //xy=946,493
AudioFilterBiquad        filterBand2; //xy=949,544
AudioFilterBiquad        filterBand3; //xy=949,595
AudioMixer4              mixerMarker1;         //xy=1261,409
AudioMixer4              mixerMarker2; //xy=1265,497
AudioMixer4              mixerMarker3; //xy=1273,584
AudioAnalyzeToneDetect   toneMarker1;    //xy=1506,410
AudioAnalyzeToneDetect   toneMarker2;    //xy=1510,498
AudioAnalyzeToneDetect   toneMarker3; //xy=1512,584
AudioConnection          patchCord1(i2s1, 0, mixerLeft, 0);
AudioConnection          patchCord2(i2s1, 1, filterMarker, 0);
AudioConnection          patchCord3(i2s1, 1, mixerRight, 0);
AudioConnection          patchCord4(filterMarker, 0, mixerMarker, 0);
AudioConnection          patchCord5(mixerLeft, filterLeft);
AudioConnection          patchCord6(mixerRight, filterRight);
AudioConnection          patchCord7(mixerMarker, filterBand1);
AudioConnection          patchCord8(mixerMarker, filterBand2);
AudioConnection          patchCord9(mixerMarker, filterBand3);
AudioConnection          patchCord10(filterRight, 0, i2s2, 1);
AudioConnection          patchCord11(filterLeft, 0, i2s2, 0);
AudioConnection          patchCord12(filterBand1, 0, mixerMarker1, 0);
AudioConnection          patchCord13(filterBand2, 0, mixerMarker2, 0);
AudioConnection          patchCord14(filterBand3, 0, mixerMarker3, 0);
AudioConnection          patchCord15(mixerMarker1, toneMarker1);
AudioConnection          patchCord16(mixerMarker2, toneMarker2);
AudioConnection          patchCord17(mixerMarker3, toneMarker3);
AudioControlSGTL5000     sgtl5000_1;     //xy=858,202
// GUItool: end automatically generated code


// For optimal detection, the target frequencies should be integer multiples of the sampling rate divided by the amount of sampled cycles
float SAMPELING_FREQUENCY = 44100; // Hz
float GOERTZEL_CYCLES = 150;
float FREQUENCY_MULTIPLES = SAMPELING_FREQUENCY / GOERTZEL_CYCLES;
float STARTING_MULTIPLE = 52.0;  // 52 --> 15288.0

struct BitState {
  bool goertzelState;
  bool debounceState;
  elapsedMillis debounceTime;
  bool markerState;
  elapsedMillis markerTime;
  int bitIdx;
  int pinNr;
  float frequency;
  float gain;
  float certainty;
  AudioFilterBiquad *bandFilter;
  AudioMixer4 *amplifier;
  AudioAnalyzeToneDetect *freqFilter;
};

BitState bits[N_BITS] = {
  {0, 0, 0, 0, 0, 0, 0, (STARTING_MULTIPLE + 0) * FREQUENCY_MULTIPLES, 4.0, 0.5, &filterBand1, &mixerMarker1, &toneMarker1},
  {0, 0, 0, 0, 0, 1, 1, (STARTING_MULTIPLE + 1) * FREQUENCY_MULTIPLES, 4.0, 0.4, &filterBand2, &mixerMarker2, &toneMarker2},
  {0, 0, 0, 0, 0, 2, 2, (STARTING_MULTIPLE + 2) * FREQUENCY_MULTIPLES, 6.0, 0.4, &filterBand3, &mixerMarker3, &toneMarker3},
};

int pins[8] = {0, 1, 2, 3, 4, 5, 6, 9};
int unusedMarkerPins[N_UNUSED_BITS] = {3, 4, 5, 6, 9};
int markerValues[8] = {0, 0, 0, 0, 0, 0, 0, 0};
uint8_t virtualRegister = 0x00;

const int SIGNAL_TONE = 0;
const int DATA_LOW = 1;
const int DATA_HIGH = 2;
elapsedMillis markerClock = 0;

// Initialize led strip (status led)
CRGB leds[N_LEDS * STRIPS];
CRGB::HTMLColorCode ON = CRGB::HotPink;
CRGB::HTMLColorCode SENDING = CRGB::DarkGreen;
CRGB::HTMLColorCode FAULT = CRGB::DarkRed;

// Declare global variables and constants
const float FILTER_FREQ = 14000.0;
const unsigned int DEBOUNCE_DELAY = 50;
const unsigned int TONE_DETECTION_DURATION = 600;

float mixerLeftGain = 0.5;
float mixerRightGain = 0.5;
float mixerMarkerGain = 2.4;

float lowpassQualityLeft1 = 5.0;
float lowpassQualityRight1 = 5.0;
float highpassQuality = 8.0;
float bandFilterQuality = 8.0;

void setup(){
  // Start serial connection
  Serial.begin(9600);

  // Initialize audio buffer and start main audio controller
  AudioMemory(20);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.5);
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);

  // Initialize lowpass filters
  filterLeft.setLowpass(0, FILTER_FREQ, lowpassQualityLeft1);
  filterRight.setLowpass(0, FILTER_FREQ, lowpassQualityRight1);

  filterLeft.setNotch(1, bits[SIGNAL_TONE].frequency);
  filterRight.setNotch(1, bits[SIGNAL_TONE].frequency);

  filterLeft.setNotch(2, bits[DATA_LOW].frequency);
  filterRight.setNotch(2, bits[DATA_LOW].frequency);

  filterLeft.setNotch(3, bits[DATA_HIGH].frequency);
  filterRight.setNotch(3, bits[DATA_HIGH].frequency);

  // filterLeft.setLowpass(1, FILTER_FREQ, lowpassQualityLeft2);
  // filterRight.setLowpass(1, FILTER_FREQ, lowpassQualityRight2);

  // Initialize highpass filter
  filterMarker.setHighpass(0, FILTER_FREQ, highpassQuality);

  // Set gains
  mixerLeft.gain(0, mixerLeftGain);  // lowpass
  mixerRight.gain(0, mixerRightGain);  // lowpass
  mixerMarker.gain(0, mixerMarkerGain);  // highpass

  // Initialize led strip (status led)
  FastLED.addLeds<STRIPS, WS2812B, DATA_PIN, GRB>(leds, N_LEDS);
  FastLED.setBrightness(40);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);

  // Set status led to pink
  FastLED.clear();
  leds[0] = ON;
  FastLED.show();

  // Initialize marker pins and corresponding frequency filters
  for (int idx = 0; idx < N_BITS; idx++){
    pinMode(bits[idx].pinNr, OUTPUT);
    bits[idx].bandFilter->setBandpass(0, bits[idx].frequency, bandFilterQuality);
    bits[idx].amplifier->gain(0, bits[idx].gain);
    bits[idx].freqFilter->frequency(bits[idx].frequency, GOERTZEL_CYCLES);
  }

  for (int idx = 0; idx < N_UNUSED_BITS; idx++){
    pinMode(unusedMarkerPins[idx], OUTPUT);
  }
};

// Sets marker bits according to the status information in the (virtual) register
void setMarkerBits(uint8_t virtualRegister){
  for(int idx = 0; idx < 8; idx++){
    digitalWriteFast(pins[idx], ((virtualRegister >> idx) & 0x01));
  }
};

void checkFreqs(){
  for (int idx = 0; idx < N_BITS; idx++){
    bits[idx].goertzelState = bits[idx].freqFilter->read() > bits[idx].certainty;
    if (bits[idx].goertzelState == bits[idx].debounceState){
      bits[idx].debounceTime = 0;
    } else if (bits[idx].debounceTime > DEBOUNCE_DELAY){
      bits[idx].debounceState = bits[idx].goertzelState;
      bits[idx].debounceTime = 0;
      bits[idx].markerTime = 0;
    }
  }
};

void loop(){
  checkFreqs();
  if (bits[SIGNAL_TONE].debounceState && markerClock > 2000){
    markerClock = 0;
    for (unsigned int idx = 0; idx < 8; idx++){
      int highCount = 0;
      int lowCount = 0;
      while (markerClock < (200 * (idx+1))){
        checkFreqs();
        if (bits[DATA_LOW].debounceState){
          lowCount++;
        } else if (bits[DATA_HIGH].debounceState){
          highCount++;
        }
      }
      if (highCount >= lowCount){
        markerValues[idx] = 1;
      } else {
        markerValues[idx] = 0;
      }
    }
    for (int i = 0; i < 8; i++){
       virtualRegister = virtualRegister | (markerValues[i] << i);
       Serial.print(markerValues[7 - i]);
    }
    Serial.println("");
    setMarkerBits(virtualRegister);
    delay(500);
    virtualRegister = 0x00;
    setMarkerBits(virtualRegister);
  }
};

