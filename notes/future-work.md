# Future

## Short to-do list

- [ ] Add rubber feet to the enclosure
- [ ] Add label to enclosure
  - `DTMF Tone Markers \n labsupport@fsw.leidenuniv.nl \n June 2024`

## Inaudible Markers

Use a [Teensy 4](https://www.pjrc.com/store/teensy40.html) and its [sound board](https://www.pjrc.com/store/teensy3_audio.html) to receive audio from a source, then filter out the marker-tones and feed the filtered audio back to the participant. Consider using a low-pass filter (e.g. at 14 kHz) and keeping the marker tones above that frequency.

[This](https://www.pjrc.com/teensy/gui/#) website can be used to design the filtering and tone detection.