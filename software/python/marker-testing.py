"""
Python script for testing marker functionality. Also used to test performance of a predefined set of audio/filter parameters.
"""
#%%
# Imports
import pydub.playback
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
from tqdm.notebook import tqdm
import pandas as pd
import soundfile as sf
import pydub

# Constants
SAMPLING_FREQUENCY = 44100
RANGE = [15000, 18000]
DURATION = 1

# Helper functions
def frequency_mapping(goertzel_cycles, n_bits=8):
    """Maps frequencies to bits given a specific amount of goertzel cycles by using [the formula explained by Kevin Banks](https://courses.cs.washington.edu/courses/cse466/11au/resources/GoertzelAlgorithmEETimes.pdf).

    Args:
        goertzel_cycles (int): The amount of cycles the goertzel algorithm will take in order to create an estimation.
        n_bits (int, optional): The amount of mappings that need to be made between bits and frequencies in the desired range. Defaults to 8.

    Returns:
        dict: A dictionary containing bit - frequency mappings.
    """
    frequency_multiples = SAMPLING_FREQUENCY / goertzel_cycles
    starting_multiple = 0
    
    while (starting_multiple * frequency_multiples) < RANGE[0]:
        starting_multiple += 1
    
    freq_mapping = {b:(starting_multiple + b) * frequency_multiples for b in range(n_bits)}
    return freq_mapping

def makesine(freq, noise=0.0):
    """Generates sinewave values for a defined frequency.

    Args:
        freq (int): The sinewave's frequency
        noise (float, optional): The percentage of white noise in the signal. In the range of 0.0 up to 1.0. Defaults to 0.0.

    Returns:
        numpy.array: A numpy array containing sinewave values for the desired signal length.
    """
    t = np.linspace(0, DURATION, math.ceil(SAMPLING_FREQUENCY * DURATION))
    y = np.sin(2 * np.pi * freq * t)
    if noise:
        assert 0 <= noise <= 1
        y *= 1 - noise
        y += np.random.normal(0, 1, math.ceil(SAMPLING_FREQUENCY * DURATION)) * noise
    return y

def create_sound(bits, freq_mapping, noise = 0.0, return_sound=False):
    """Combines sinewaves for different bits in order to create a single marker sound.

    Args:
        bits (list[int]): A list containing the bits that need to be combined into a single marker sound.
        freq_mapping (dict): A mapping between bits and frequencies.
        noise (float, optional): The percentage of white noise in the signal. In the range of 0.0 up to 1.0. Defaults to 0.0.
        return_sound (bool, optional): Flag defining if sinevalues or a playable sound widget should be returned. Defaults to False (sinewave).

    Returns:
        np.array: Array containing values for the combined sinewaves.
    """
    assert len(bits) <= 4   # TODO: Generalize for eight bits
    gain = [None, 0.3, 0.3, 0.11, 0.07][len(bits)]
    sine = 0
    for b in bits:
        sine += makesine(freq_mapping[b], 0.0) * gain
        
    if noise:
        sine += np.random.normal(0, 1, math.ceil(SAMPLING_FREQUENCY * DURATION)) * noise
        
    if return_sound:
        return ipd.Audio(sine, rate=SAMPLING_FREQUENCY, normalize = False, autoplay=True)
    else:
        return sine
    
def connect():
    """Creates a serial connection to the (prototype) tonemarker device.

    Returns:
        serial.Serial: A serial class for the tonemarker device.
    """
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
    
def collect_response(sound, device, mp3=False):
    """Plays a sound and then collects a response from the serial device.

    Args:
        sound (numpy.array): A numpy array with sine wave values.
        device (serial.Serial): Serial class for the tonemarker device.
        mp3 (bool, optional): Flag describing if the sound should be converted to mp3 before it is played. Defaults to False.

    Returns:
        numpy.array: A numpy array containing all unique responses from the serial device.
    """
    if not mp3:
        sd.play(sound, SAMPLING_FREQUENCY)
        sd.wait()
        
    else:
        sf.write("./tmp.wav", sound, SAMPLING_FREQUENCY)
        segment = pydub.AudioSegment.from_wav("./tmp.wav")
        segment = segment - 2
        segment.export("./tmp.mp3", format='mp3')
        sounds = pydub.AudioSegment.from_mp3("./tmp.mp3")
        pydub.playback.play(sounds)
        sleep(1)
        os.remove("./tmp.wav")
        os.remove("./tmp.mp3")
        
    response = list()
    while device.in_waiting:
        sleep(.1)
        response.append(device.readline().decode().strip())
        
    device.flush()
    return np.unique(response)

def test_markers(bits, freq_mapping, device, noise = 0.0, mp3=False):
    """Tests responses for all possible combinations of the specified bits and returns an array with the accuracy of each combination.

    Args:
        bits (list[int]): List with each bit that is to be tested.
        freq_mapping (dict): Mapping between bits and frequencies.
        device (serial.Serial): Serial class for the tonemarker device.
        noise (float, optional): Percentage of white noise in the signal. Ranges between 0.0 and 1.0. Defaults to 0.0.
        mp3 (bool, optional): Flag describing if the sound should be converted to mp3 before it is played. Defaults to False.

    Returns:
        numpy.array: Numpy array with accuracies where each index corresponds to a specific combination of bits.
    """
    counter = np.zeros(2**len(bits) - 1)
    device.flush()
    for idx, p in enumerate(chain.from_iterable(combinations(bits, r) for r in range(1, len(bits) + 1))):
            correct = ''.join('1' if i in p else '0' for i in range(8))
            sound = create_sound(p, freq_mapping, noise)
            response = collect_response(sound, device, mp3)
            if len(response) == 2 and response[-1] == correct:
                counter[idx] += 1
                
    return counter

def tune_parameters(bits, tune_ranges, n_repetitions, device, noise = 0.0):
    """Tests the performance of provided device parameters for a specified amount of repetitions.

    Args:
        bits (list[int]): The bits that need to be tested.
        tune_ranges (dict): Dictionary containing parameter values over which tests need to be performed.
        n_repetitions (int): Amount of repetitions for each parameter combination.
        device (serial.Serial): Serial class for the tonemarker device.
        noise (float, optional): Percentage of white noise in the signal. Ranges between 0.0 and 1.0. Defaults to 0.0.
    """
    cases = list(product(*tuple(tune_ranges.values())))
    n_tests = 2**len(bits) - 1
    
    if not os.path.exists(OUTFILE):
        with open(OUTFILE, 'w') as fo:
            fo.write(f"accuracy, noise, gain0, gain1, gain2, gain3, gain4, gain5, gain6, gain7, thresh0, thresh1, thresh2, thresh3, thresh4, thresh5, thresh6, thresh7, {', '.join([x.replace(',', '') for x in map(str, chain.from_iterable(combinations(bits, r) for r in range(1, len(bits) + 1)))])}\n")
            fo.close()
            
    for nr, case in enumerate(cases):
        mixerMarker = case[0]
        high = case[1]
        band = case[2]
        goertzel = case[3]
        gains = case[4]
        certainties = case[5]
        
        freq_map = frequency_mapping(goertzel)
        
        device.write(f"{' '.join(map(str, freq_map.values()))}\n".encode())
        sleep(.1)
        device.write(f"{mixerMarker} {high} {band} {goertzel}\n".encode())
        sleep(.1)
        device.write(f"{' '.join(map(str, gains))}\n".encode())
        sleep(.1)
        device.write(f"{' '.join(map(str, certainties))}\n".encode())
        sleep(.1)
        
        accuracy = np.zeros(n_tests)
        for _ in tqdm(range(n_repetitions), desc = f"Testcase {nr+1}/{len(cases)}"):
            results = test_markers(bits, freq_map, device, noise)
            accuracy += results
            
        accuracy /= n_repetitions
        with open(OUTFILE, 'a') as fo:
            fo.write(f"{np.sum(accuracy) / n_tests}, {noise}, {', '.join(map(str, gains))}, {', '.join(map(str, certainties))}, {', '.join(map(str, accuracy))}\n")
            
def test_compression(bits, params, n_repetitions, device) -> None:
    """Test parameter performance on markers from a compressed file format (mp3).

    Args:
        bits (list[int]): The bits that need to be tested.
        params (dict): Dictionary containing parameter values over which tests need to be performed.
        n_repetitions (int): Amount of repetitions for each parameter combination.
        device (serial.Serial): Serial class for the tonemarker device.
    """
    if not os.path.exists(OUTFILE):
        with open(OUTFILE, 'w') as fo:
            fo.write(f"accuracy, repetitions, lower range, {', '.join([x.replace(',', '') for x in map(str, chain.from_iterable(combinations(bits, r) for r in range(1, len(bits) + 1)))])}\n")
            fo.close()
            
    mixerMarker = params["mixerMarker"]
    high = params["high"]
    band = params["band"]
    goertzel = params["goertzel"]
    gains = params["gains"]
    certainties = params["certainties"]
    
    freq_map = frequency_mapping(goertzel)
    
    device.write(f"{' '.join(map(str, freq_map.values()))}\n".encode())
    sleep(.1)
    device.write(f"{mixerMarker} {high} {band} {goertzel}\n".encode())
    sleep(.1)
    device.write(f"{' '.join(map(str, gains))}\n".encode())
    sleep(.1)
    device.write(f"{' '.join(map(str, certainties))}\n".encode())
    sleep(.1)
    
    n_tests = 2**len(bits) - 1
    accuracy = np.zeros(n_tests)
    for _ in tqdm(range(n_repetitions), desc = "Mp3 testing"):
        results = test_markers(bits, freq_map, device, noise, mp3=True)
        accuracy += results
        
    accuracy /= n_repetitions
    with open(OUTFILE, 'a') as fo:
        fo.write(f"{np.sum(accuracy) / n_tests}, {n_repetitions}, {RANGE[0]}, {', '.join(map(str, accuracy))}\n")
           
#%%
if __name__ == "__main__":
    OUTFILE = os.path.abspath("../../data/attenuation-reliability-test.txt")
    teensy = connect()
    n_bits = 4
    bits = list(range(n_bits))
    noise = 0.0
    
    # Don't tune the last 4 bits in order to reduce tuning time
    tune_ranges = {
        "mixerMarker" : tuple([2.4]),
        "high": tuple([8.0]),
        "band": tuple([8.0]),
        "goertzel": tuple([150]),
        "gains": tuple([[4, 4, 6, 6, 6, 6, 6, 6], [6, 6, 6, 6, 6, 6, 6, 6]]),
        "certainties": tuple([[.5, .5, .5, .5, .5, .5, .5, .5]])
    }
    
    params = {
        "mixerMarker" : 2.4,
        "high": 8.0,
        "band": 8.0,
        "goertzel": 150,
        "gains": [4, 4, 6, 6, 6, 6, 6, 6],
        "certainties": [.5, .5, .5, .5, .5, .5, .5, .5]
    }
    
    for lower_range in [15000]:
        RANGE = [lower_range, 18000]
        
        # tune_parameters(bits, tune_ranges, 10, teensy, noise)
        test_compression(bits, params, 100, teensy)

#%%

df = pd.read_csv(OUTFILE)
df.round(3)
