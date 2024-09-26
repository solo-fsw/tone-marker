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
DURATION = .2
GOERTZEL = 150

OUTDIR = os.path.abspath("../../data/new")

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
    # sine = add_ramps(sine)
    if return_sound:
        return ipd.Audio(sine, rate=SAMPLING_FREQUENCY, normalize = False, autoplay=True)
    else:
        return sine
    
def add_ramps(sine):
    length = int((SAMPLING_FREQUENCY//1000) * DURATION)
    ramps = np.ones(8820)
    ramps[:100] = np.linspace(0, 1, 100)
    ramps[-100:] = np.linspace(1, 0, 100)
    return sine * ramps
    
def create_sounds(bits, freq_mapping):
    sounds = dict()
    for idx, p in enumerate(chain.from_iterable(combinations(bits, r) for r in range(1, len(bits) + 1))):
        bitstring = ''.join('1' if i in p else '0' for i in range(8))[::-1]
        sound = create_sound(p, freq_mapping)
        sounds[str(int(bitstring, 2))] = sound
        print(bitstring, str(int(bitstring, 2)))
        
    return sounds

def create_signal(bitstr):
    data_high = create_sound([0, 2], freq_mapping)
    data_low = create_sound([0, 1], freq_mapping)
    
    final = list()
    for c in bitstr:
        if str(c) == "1":
            final.extend(data_high)
        elif str(c) == "0":
            final.extend(data_low)
            
    return np.array(final)

def create_silence():
    
    silence = create_sound([3, 4], freq_mapping) * 0
    
    final = list()
    for _ in range(8):
        final.extend(silence)
        
    return np.array(final)


#%%

if __name__ == "__main__":
    n_bits = 4
    bits = list(range(n_bits))
    
    freq_mapping = frequency_mapping(GOERTZEL)
    
    signal = create_sound([0], freq_mapping)
    
    # bitstr = create_signal([1, 1, 1, 1, 1, 1, 1, 0][::-1])
    bitstr = [
        *create_signal([1, 1, 1, 1, 1, 1, 1, 1][::-1]),
        *create_silence(),
        *create_signal([1, 1, 1, 1, 1, 1, 1, 0][::-1]),
        *create_silence(),
        *create_signal([1, 1, 1, 1, 1, 1, 0, 0][::-1]),
        *create_silence(),
        *create_signal([1, 1, 1, 1, 1, 0, 0, 0][::-1]),
        *create_silence(),
        *create_signal([1, 1, 1, 1, 0, 0, 0, 0][::-1]),
        *create_silence(),
        *create_signal([1, 1, 1, 0, 0, 0, 0, 0][::-1]),
        *create_silence(),
        *create_signal([1, 1, 0, 0, 0, 0, 0, 0][::-1]),
        *create_silence(),
        *create_signal([1, 0, 0, 0, 0, 0, 0, 0][::-1]),
    ]
    
    if not os.path.isdir(OUTDIR):
        os.mkdir(OUTDIR)
    
    sf.write(f"{os.path.join(OUTDIR, 'bitstr')}.wav", bitstr, SAMPLING_FREQUENCY)
    
    marker_value = "bitstr"
    
    segment = pydub.AudioSegment.from_wav(f"{os.path.join(OUTDIR, marker_value)}.wav")
    
    quieter_segment = segment - 6
    mp3_segment = segment - 2
    
    quieter_segment.export(f"{os.path.join(OUTDIR, marker_value)}.wav", format='wav')
    mp3_segment.export(f"{os.path.join(OUTDIR, marker_value)}.mp3", format='mp3')

        
# %%
