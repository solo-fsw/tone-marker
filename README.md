# Tone Markers

![The Tone Marker device](./readme-media/toneMarkerDevice.png)  <!-- TODO -->

## Device purpose

The purpose of the "Tone Marker" device is to translate high-frequency tones ($\ge$ 15KHz) into Transistor-transistor Logic (TTL) markers. This allows any application with an audio output (e.g. virtual reality games, websites, cellphones) to send custom markers (values ranging from 0 up to 255) by embedding these marker tones in the audio output.

Sending markers is necessary for synchronization of recorded data (e.g. BIOPAC measurements) with VR experiences, where hardware control is not possible.

## Usage

### Setup

Internally, the Tone Marker device consists of a Teesy 4.0 ([Teensy 4.0](https://www.pjrc.com/store/teensy40.html)) and a custom made PCB audioboard. <!-- TODO: Add PCB image?--> 
Power is provided through a Mini-USB connection to a dedicated power source (such as a powerbank). The audio input (from e.g. the VR glasses) is fed into the tone-marker, while simultaneously being fed directly into the marker destination (for quality control). Marker output is done through a Female DB25 connector, which is then connected to the required data recording device (e.g. BIOPAC, BIOSEMI).

An overview of the internal and external connections is shown below.

![An overview of the connection in / to the Tone Marker device](./readme-media/tonemarker-diagram.jpg)

### Communication protocol

The device expects tones (or combinations of tones) of specific frequencies. The device is currently optimized for performance on the first four bits (marker values 0 through 15).

Frequencies are detected by the goertzel algorithm, and are therefore optimized for their respective goertzel bin. This is done for the goertzel algorithm using 150 clock cycles and a sampling frequency of 44100, resulting in the following frequencies:

- **bit 0**: 15288.0 Hz
- **bit 1**: 15582.0 Hz
- **bit 2**: 15876.0 Hz
- **bit 3**: 16170.0 Hz

Soundfiles for these marker values can be found in this repository (in the `media` directory). Usage of the `.wav` files is recommended, since they are less compressed than the `.mp3` files.

## Notes

### Power supply

Some USB power adapters have some noise, which is then audible in the audio signal. In order to avoid this, the usage of a powerbank for powering the Tone Marker device is recommended.

### Volume

Different values of input audio volume might affect Tone Marker performance in different ways. The currently recommended volume level is 65% (on a windows PC). Marker functionality can be monitored by checking the green LEDs.

### Additional information / sources

- [Information on the Teensy 4.0 audioshield](https://forum.pjrc.com/index.php?threads/available-teensy-4-0-pins-when-audio-shield-d-attached.58331/)
- [Information on available Teensy pins while the audioshield is connected](https://github.com/luni64/TeensyTimerTool/wiki/Avoid-PWM-timer-clashes)
- [Teensy pin-register mapping](https://forum.pjrc.com/index.php?threads/teensy-4-1-digital-i-o-pin-map.64226/)
- [Using Teensy GPIO registers](https://forum.pjrc.com/index.php?threads/tutorial-on-digital-i-o-atmega-pin-port-ddr-d-b-registers-vs-arm-gpio_pdir-_pdor.17532/)
- [DigitalRead, DigitalWrite, DigitalWriteFast speeds](https://forum.pjrc.com/index.php?threads/speed-of-digitalread-and-digitalwrite-with-teensy3-0.24573/)
- [Additional explanation on registers](https://forum.pjrc.com/index.php?threads/unclear-on-how-to-use-ddrx-and-portx-teensy-3-2.53950/)
- [Goertzel Algorithm](https://courses.cs.washington.edu/courses/cse466/11au/resources/GoertzelAlgorithmEETimes.pdf)
- [Changing soundfile audio volume in python](https://stackoverflow.com/questions/43679631/python-how-to-change-audio-volume)
