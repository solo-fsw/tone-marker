#%%
from pprint import pp
import math
import numpy as np
import IPython.display as ipd
from matplotlib import pyplot as plt
from scipy.fft import fft, fftfreq, fftshift

STARTING_MULTIPLE = 73

SAMPLING_FREQUENCY = 44100
GOERTZEL_CYCLES = 200
FREQUENCY_MULTIPLES = SAMPLING_FREQUENCY / GOERTZEL_CYCLES

PLT_CUTOFF = 400
DURATION = .5

freq_mapping = {b:(STARTING_MULTIPLE + b) * FREQUENCY_MULTIPLES for b in range(8)}
pp(freq_mapping)

#%%
def makesine(freq, duration):
    t = np.linspace(0, duration, math.ceil(SAMPLING_FREQUENCY * duration))
    y = np.sin(2 * np.pi * freq * t)
    return y

#%%
sinewave1 = makesine(freq_mapping[0], DURATION)
sinewave2 = makesine(freq_mapping[1], DURATION)
sinewave3 = makesine(freq_mapping[2], DURATION)
sinewave4 = makesine(freq_mapping[3], DURATION)
sinewave12 = sinewave1 + sinewave2

ipd.Audio(sinewave1 * 0.3, rate=SAMPLING_FREQUENCY, normalize = False)
# ipd.Audio(sinewave2 * 0.3, rate=SAMPLING_FREQUENCY, normalize = False)
# ipd.Audio(sinewave12 * 0.10, rate=SAMPLING_FREQUENCY, normalize = False)

#%%
t = np.linspace(0, DURATION, math.ceil(SAMPLING_FREQUENCY * DURATION))[:PLT_CUTOFF]

fig = plt.figure(1)
ax1 = fig.add_subplot(311)
ax1.plot(t, sinewave1[:PLT_CUTOFF])
ax2 = fig.add_subplot(312)
ax2.plot(t, sinewave2[:PLT_CUTOFF])
ax3 = fig.add_subplot(313)
ax3.plot(t, sinewave12[:PLT_CUTOFF])
plt.show()

#%%

N = len(t)
T = 1.0 / SAMPLING_FREQUENCY
x = np.linspace(0.0, N*T, N, endpoint=False)
y = sinewave12

yf = fft(y)
xf = fftfreq(N, T)

yf = yf[0:N//2]
xf = xf[:N//2]

mask = (xf > 15000) & (xf < 18000)
xf = xf[mask]
yf = yf[mask]

plt.plot(xf, 2.0/N * np.abs(yf))
plt.grid()
plt.show()

# # %%
# import pyaudio
# from scipy.io.wavfile import write

# p = pyaudio.PyAudio()
# info = p.get_host_api_info_by_index(0)
# n_devices = info.get("deviceCount")

# for i in range(n_devices):
#     if (p.get_device_info_by_host_api_device_index(0, i).get('maxInputChannels')) > 0:
#         print(i, " - ", p.get_device_info_by_host_api_device_index(0, i).get("name"))
#         if "Teensy" in p.get_device_info_by_host_api_device_index(0, i).get('name'):
#             device_idx = i

# #%%
            
# CHUNKSIZE = 1024
# SECONDS = 5
            
# stream = p.open(
#     input_device_index=device_idx,
#     input=True,
#     rate= 44100,
#     channels=2,
#     format=pyaudio.paFloat32,
# )

# frames = list()
# for _ in range(0, SAMPLING_FREQUENCY // CHUNKSIZE * SECONDS):
#     data = stream.read(CHUNKSIZE)
#     frames.append(np.fromstring(data, dtype=np.float32))
    
# npdata = np.hstack(frames)

# stream.stop_stream()
# stream.close()
# p.terminate()

# write("test.wav", SAMPLING_FREQUENCY, npdata)

# with open("../../data/tune_results_extra.txt", 'r') as fo:
#     with open("../../data/tune_results.txt", 'r') as fo2:
#         counts = fo.readlines()
#         for line in fo2.readlines()[1:]:
#             params = [float(x.strip()) for x in line.split(',')]
#             idx = int(params[0])
#             cnt = counts[idx:idx+10]
#             c = str([x.split(" - ")[1].strip() for x in cnt])
#             c = np.array([int(x) for x in re.findall("\d+", c)])
#             with open("../../data/accuracy.txt", "a") as fo3:
#                 fo3.write(f"{np.sum(c) / 150}, {params[3]}, {params[4]}, {params[5]}, {params[6]}\n")
