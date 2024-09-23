#%%
import math
import numpy as np
from itertools import combinations, chain
import IPython.display as ipd
import os
import soundfile as sf
import pydub

SAMPLING_FREQUENCY = 44100
RANGE = [15000, 18000]
DURATION = 1
GOERTZEL = 150

OUTDIR = os.path.abspath("../../data/soundfiles")

#%%
def frequency_mapping(goertzel_cycles, n_bits=8):
    frequency_multiples = SAMPLING_FREQUENCY / goertzel_cycles
    starting_multiple = 0
    
    while (starting_multiple * frequency_multiples) < RANGE[0]:
        starting_multiple += 1
    
    freq_mapping = {b:(starting_multiple + b) * frequency_multiples for b in range(n_bits)}

    return freq_mapping

def makesine(freq):
    t = np.linspace(0, DURATION, math.ceil(SAMPLING_FREQUENCY * DURATION))
    y = np.sin(2 * np.pi * freq * t)
    return y

def create_sound(bits, freq_mapping, return_sound=False):
    assert len(bits) <= 4   # TODO: Generalize for eight bits
    gain = [None, 1.0, 0.5, 0.33, 0.25][len(bits)]
    sine = 0
    for b in bits:
        sine += makesine(freq_mapping[b]) * gain
        
    sine = add_ramps(sine)
    if return_sound:
        return ipd.Audio(sine, rate=SAMPLING_FREQUENCY, normalize = False, autoplay=True)
    else:
        return sine
    
def add_ramps(sine, duration=100):
    length = (SAMPLING_FREQUENCY//1000) * duration
    ramps = np.ones(SAMPLING_FREQUENCY)
    ramps[:length] = np.linspace(0, 1, length)
    ramps[-length:] = np.linspace(1, 0, length)
    return sine * ramps
    
def create_sounds(bits, freq_mapping):
    sounds = dict()
    for idx, p in enumerate(chain.from_iterable(combinations(bits, r) for r in range(1, len(bits) + 1))):
        bitstring = ''.join('1' if i in p else '0' for i in range(8))[::-1]
        sound = create_sound(p, freq_mapping)
        sounds[str(int(bitstring, 2))] = sound
        print(bitstring, str(int(bitstring, 2)))
        
    return sounds


#%%

if __name__ == "__main__":
    n_bits = 4
    bits = list(range(n_bits))
    
    freq_mapping = frequency_mapping(GOERTZEL)
    sounds = create_sounds(bits, freq_mapping)
    
    if not os.path.isdir(OUTDIR):
        os.mkdir(OUTDIR)
    
    for marker_value, marker_tone in sounds.items():
        sf.write(f"{os.path.join(OUTDIR, marker_value)}.wav", marker_tone, SAMPLING_FREQUENCY)
        
        segment = pydub.AudioSegment.from_wav(f"{os.path.join(OUTDIR, marker_value)}.wav")
        
        quieter_segment = segment - 6
        mp3_segment = segment - 2
        
        quieter_segment.export(f"{os.path.join(OUTDIR, marker_value)}.wav", format='wav')
        mp3_segment.export(f"{os.path.join(OUTDIR, marker_value)}.mp3", format='mp3')

        