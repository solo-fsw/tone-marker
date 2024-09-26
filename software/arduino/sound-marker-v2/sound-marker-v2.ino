#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <FastLED.h>

#define DATA_PIN 17
#define N_LEDS 1
#define STRIPS 1
#define N_BITS 4
#define N_UNUSED_BITS 4

// Begin automatically generated code
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
// End automatically generated code

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
  {0, 0, 0, 0, 0, 1, 1, (STARTING_MULTIPLE + 1) * FREQUENCY_MULTIPLES, 4.0, 0.5, &filterBand2, &mixerMarker2, &toneMarker2},
  {0, 0, 0, 0, 0, 2, 2, (STARTING_MULTIPLE + 2) * FREQUENCY_MULTIPLES, 6.0, 0.5, &filterBand3, &mixerMarker3, &toneMarker3},
  {0, 0, 0, 0, 0, 3, 3, (STARTING_MULTIPLE + 3) * FREQUENCY_MULTIPLES, 6.0, 0.5, &filterBand4, &mixerMarker4, &toneMarker4}
};

int unusedMarkerPins[N_UNUSED_BITS] = {4, 5, 6, 9};

bool done;
uint8_t virtualRegister = 0x00;

// Initialize led strip (status led)
CRGB leds[N_LEDS * STRIPS];

// Declare global variables and constants
const float FILTER_FREQ = 14000.0;
const unsigned int DEBOUNCE_DELAY = 300;
const unsigned int TONE_DETECTION_DURATION = 600;

float mixerLeftGain = 0.5;
float mixerRightGain = 0.5;
float mixerMarkerGain = 2.4;

float lowpassQualityLeft1 = 1.0;
float lowpassQualityLeft2 = 1.0;
float lowpassQualityRight1 = 1.0;
float lowpassQualityRight2 = 1.0;
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

  filterLeft.setLowpass(1, FILTER_FREQ, lowpassQualityLeft2);
  filterRight.setLowpass(1, FILTER_FREQ, lowpassQualityRight2);

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
  leds[0] = CRGB::HotPink;
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
  for(int idx = 0; idx < N_BITS; idx++){
    digitalWriteFast(bits[idx].pinNr, ((virtualRegister >> idx) & 0x01));
  }
};

void loop(){

  done = true;

  for (int idx = 0; idx < N_BITS; idx++){
    bits[idx].goertzelState = bits[idx].freqFilter->read() > bits[idx].certainty;
    if (bits[idx].goertzelState == bits[idx].debounceState){
      bits[idx].debounceTime = 0;
    } else if (bits[idx].debounceTime > DEBOUNCE_DELAY){
      bits[idx].debounceState = bits[idx].goertzelState;
      bits[idx].debounceTime = 0;
      bits[idx].markerTime = 0;
    }

    if (bits[idx].markerTime > TONE_DETECTION_DURATION){
      bits[idx].markerState = bits[idx].debounceState;
      // virtualRegister = (virtualRegister & ~(1 << bits[idx].bitIdx)) | (bits[idx].markerState << bits[idx].bitIdx);
      virtualRegister = virtualRegister | (bits[idx].markerState << bits[idx].bitIdx);
    }

    if (bits[idx].debounceState == true){
      done = false;
    }
  }

  if (done){
    setMarkerBits(virtualRegister);
    virtualRegister = 0x00;
    // TODO: How long should a marker be?
    delay(500);
    setMarkerBits(virtualRegister);
  }

  // TODO: Broke serial output!
  if (Serial.available()){
    processCommand();
  }
};

// TODO: FIX!
void processCommand(){
  // Function for processing serial commands used during parameter tuning.
  FastLED.clear();
  leds[0] = CRGB::DarkRed;
  FastLED.show();

  String command = Serial.readStringUntil('\n');
  char current;
  String val;
  unsigned int prev = 0;
  unsigned int cnt = 0;
  for(unsigned int i = 0; i < command.length(); i++){
    current = command.charAt(i);
    if (current == ' '){
      val = command.substring(prev, i);
      prev = i+1;
      bits[cnt].frequency = val.toFloat();
      cnt++;
    }
  }
  val = command.substring(prev, command.length());
  bits[cnt].frequency = val.toFloat();

  command = Serial.readStringUntil('\n');
  prev = 0;
  cnt = 0;
  for(unsigned int i = 0; i < command.length(); i++){
    current = command.charAt(i);
    if (current == ' '){
      val = command.substring(prev, i);
      prev = i+1;
      if(cnt == 0){
        mixerMarker.gain(0, val.toFloat());
        cnt++;
      } else if (cnt == 1){
        filterMarker.setHighpass(0, FILTER_FREQ, val.toFloat());
        cnt++;
      } else if (cnt == 2){
          for(int idx = 0; idx < N_BITS; idx++){
            bits[idx].bandFilter->setBandpass(0, bits[idx].frequency, val.toFloat());
          }
      }
    }
  }
  val = command.substring(prev, command.length());
  for(int idx = 0; idx < N_BITS; idx++){
    bits[idx].freqFilter->frequency(bits[idx].frequency, val.toFloat());  // 200 cycles = 4.5 ms window length
  }

  command = Serial.readStringUntil('\n');
  prev = 0;
  cnt = 0;
  for(unsigned int i = 0; i < command.length(); i++){
    current = command.charAt(i);
    if (current == ' '){
      val = command.substring(prev, i);
      prev = i+1;
      bits[cnt].gain = val.toFloat();
      cnt++;
      }
    }
  val = command.substring(prev, command.length());
  bits[cnt].gain = val.toFloat();

  for(int idx = 0; idx < N_BITS; idx++){
    bits[idx].amplifier->gain(0, bits[idx].gain);
  }

  command = Serial.readStringUntil('\n');
  prev = 0;
  cnt = 0;
  for(unsigned int i = 0; i < command.length(); i++){
    current = command.charAt(i);
    if (current == ' '){
      val = command.substring(prev, i);
      prev = i+1;
      bits[cnt].certainty = val.toFloat();
      cnt++;
      }
    }
  val = command.substring(prev, command.length());
  bits[cnt].certainty = val.toFloat();

  FastLED.clear();
  leds[0] = CRGB::HotPink;
  FastLED.show();
};
