#%%
import serial
import re
from serial.tools.list_ports import comports
import math
import numpy as np
from time import sleep
import sounddevice as sd
from itertools import combinations, chain, product
import IPython.display as ipd
import os
from pprint import pp
from tqdm.notebook import tqdm

SAMPLING_FREQUENCY = 44100
RANGE = [16000, 18000]
DURATION = .5
# STARTING_MULTIPLE = 73
# GOERTZEL_CYCLES = 200
# FREQUENCY_MULTIPLES = SAMPLING_FREQUENCY / GOERTZEL_CYCLES

#%%
def frequency_mapping(goertzel_cycles, n_bits=8):
    frequency_multiples = SAMPLING_FREQUENCY / goertzel_cycles
    starting_multiple = 0
    
    while (starting_multiple * frequency_multiples) < RANGE[0]:
        starting_multiple += 1
    
    freq_mapping = {b:(starting_multiple + b) * frequency_multiples for b in range(n_bits)}
    print(f"Goertzel cycles: {goertzel_cycles}")
    pp(freq_mapping)
    return freq_mapping

def makesine(freq):
    t = np.linspace(0, DURATION, math.ceil(SAMPLING_FREQUENCY * DURATION))
    y = np.sin(2 * np.pi * freq * t)
    return y

def create_sound(bits, freq_mapping, return_sound=False):
    gain = [None, 0.3, 0.3, 0.11, 0.07][len(bits)]  # TODO: Generalize for eight bits
    sine = 0
    for b in bits:
        sine += makesine(freq_mapping[b]) * gain
        
    if return_sound:
        return ipd.Audio(sine, rate=SAMPLING_FREQUENCY, normalize = False, autoplay=True)
    else:
        return sine
    
def connect():
    ser = serial.Serial()
    connected = False
    for port, desc, hwid in comports():
        if re.match("USB VID:PID=16C0:0483", hwid):
            ser = serial.Serial(port, baudrate=9600)
            connected = True
            break
            
    if connected and not ser.is_open:
        try:
            ser.open()
            assert ser.is_open
            return ser
        except:
            return None  
    elif not connected:
        return None
    elif connected and ser.is_open:
        return ser
    
def collect_response(sound, device):
    sd.play(sound, SAMPLING_FREQUENCY)
    sd.wait()
    response = list()
    while device.in_waiting:
        sleep(.1)
        response.append(device.readline().decode().strip())
        
    device.flush()
    return np.unique(response)

def test_markers(bits, freq_mapping, device):
    counter = np.zeros(2**len(bits) - 1)
    device.flush()
    for idx, p in enumerate(chain.from_iterable(combinations(bits, r) for r in range(1, len(bits) + 1))):
            correct = ''.join('1' if i in p else '0' for i in range(8))
            sound = create_sound(p, freq_mapping)
            response = collect_response(sound, device)
            if len(response) == 2 and response[-1] == correct:
                counter[idx] += 1
                
    return counter

def tune_parameters(bits, tune_ranges, n_repetitions, device):
    cases = list(product(*tuple(tune_ranges.values())))
    n_tests = 2**len(bits) - 1
    
    if not os.path.exists(OUTFILE):
        with open(OUTFILE, 'w') as fo:
            fo.write(f"accuracy, mixerMarkerGain, highpassQual, bandpassQual, goertzel, {', '.join([x.replace(',', '') for x in map(str, chain.from_iterable(combinations(bits, r) for r in range(1, len(bits) + 1)))])}\n")
            fo.close()
        
    print(f"Testing {len(cases)} cases in {n_tests} tests for {n_repetitions} repetitions.")
    
    for nr, case in enumerate(cases):
        mixerMarker = case[0]
        high = case[1]
        band = case[2]
        goertzel = case[3]
        
        freq_map = frequency_mapping(goertzel)
        
        device.write(f"{' '.join(map(str, freq_map.values()))}\n".encode())
        sleep(.1)
        device.write(f"{mixerMarker} {high} {band} {goertzel}\n".encode())
        sleep(.1)
        
        accuracy = np.zeros(n_tests)
        for _ in tqdm(range(n_repetitions)):
            results = test_markers(bits, freq_map, device)
            accuracy += results
            
        accuracy /= n_repetitions
        with open(OUTFILE, 'a') as fo:
            fo.write(f"{np.sum(accuracy) / n_tests}, {mixerMarker}, {high}, {band}, {goertzel}, {', '.join(map(str, accuracy))}\n")
        print(nr, accuracy)
        print()
           
#%%

OUTFILE = os.path.abspath("../../data/tuning.txt")

if __name__ == "__main__":
    teensy = connect()
    n_bits = 4
    bits = list(range(n_bits))
    tune_ranges = {
        "mixerMarker" : tuple([2.4]),
        "high": tuple([8.0]),
        "band": tuple([8.0]),
        "goertzel": tuple([150])
    }
    
    tune_parameters(bits, tune_ranges, 100, teensy)

#%%

import pandas as pd

df = pd.read_csv(OUTFILE)
df.round(3)