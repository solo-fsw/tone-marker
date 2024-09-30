#%%
import numpy as np
import os
from moviepy.editor import VideoClip, concatenate_videoclips, VideoFileClip, AudioFileClip, concatenate_audioclips
from pydub import AudioSegment
import time

from PIL import Image
from PIL import ImageFont
from PIL import ImageDraw

INPUT_DIR = os.path.abspath(r"../../media/wav")
OUTPUT_DIR = r"./"
size = 1920//11

TEST_VALUES = [2**x for x in range(8)]

for val in TEST_VALUES:
    soundfile = os.path.join(INPUT_DIR, f"{val}.wav")
    original_segment = AudioSegment.from_file(soundfile)
    silence_segment = AudioSegment.silent(duration=3800)
    video_audio = original_segment + silence_segment
    video_audio += 5
    video_audio.export(f"./sound{val}.wav", format="wav")
    
    img = Image.new('RGB', (1920, 1080), "white")
    font = ImageFont.truetype("arial.ttf", size=50)
    draw = ImageDraw.Draw(img)
    draw.text(
        (1920//2 - 180, 1080//4),
        f"Marker value: {val}",
        (0, 0, 0),
        font=font
    )
    
    
    for idx, b in enumerate(f"{val:08b}"):
        if b == "1":
            colour = "limegreen"
        else:
            colour = "grey"
        draw.circle(
            ((2+idx)*size, 1080//2),
            30,
            colour
        )
    
    def make_frame(t):
        return np.array(img)
        
    clip = VideoClip(make_frame, duration=5)
    clip = clip.set_audio(AudioFileClip(f"./sound{val}.wav"))
    clip.write_videofile(f"./movie{val}.mp4", fps=24)

cliparray = [VideoFileClip(f"./movie{val}.mp4") for val in TEST_VALUES]
audioarray = [AudioFileClip(f"./sound{val}.wav") for val in TEST_VALUES]
    
final_audio = concatenate_audioclips(audioarray)
final = concatenate_videoclips(cliparray, method="chain")
final = final.set_audio(final_audio)
final.write_videofile("./movie.mp4", fps=24, audio_codec="aac")

time.sleep(2)

# %%

for  val in [2**x for x in range(8)]:
    os.remove(f"./sound{val}.wav")
    os.remove(f"./movie{val}.mp4")
    