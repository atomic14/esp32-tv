import subprocess
import soundfile as sf
from tqdm import tqdm
import tempfile
import shutil
import numpy as np
import cv2
import hashlib
import os
from scipy.signal import resample
import pickle
import gzip

from .image_utils import resize_cover


def extract_audio(video_file, sample_rate=16000):
    # create a named temporary file to store the audio
    with tempfile.NamedTemporaryFile(suffix=".wav") as f:
        # check that ffmpeg exists
        if not shutil.which("ffmpeg"):
            raise Exception("ffmpeg not installed!")
        # convert the video file to wav format
        status = subprocess.call(
            ["ffmpeg", "-i", video_file, "-q:a", "0", "-map", "a", f.name, "-y"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.STDOUT,
        )
        if status != 0:
            raise Exception("ffmpeg returned a non-zero status: {}".format(status))
        # Read the audio file
        audio_data, sample_rate = sf.read(f.name)
        # Check if the audio is stereo
        if len(audio_data.shape) == 2 and audio_data.shape[1] == 2:
            # Convert to mono by averaging the two channels
            audio_data = np.mean(audio_data, axis=1)
        # Resample to 16kHz
        resampled_audio = resample(
            audio_data, int(len(audio_data) * 16000 / sample_rate)
        )
        # normalize the audio
        resampled_audio = resampled_audio / np.max(np.abs(resampled_audio))
        # Convert to signed 8-bit
        audio_8bit = np.int8(resampled_audio * 127)
        # Create audio buffer
        audio_buffer = audio_8bit.tobytes()
        # we've now got 16KHz 8-bit mono audio in a buffer
        return audio_buffer


def extract_video_frames(video_file, target_size, frame_rate=15):
    video_jpegs = []
    cap = cv2.VideoCapture(video_file)
    # get the total number of frames
    total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    frame_interval = 1000 / frame_rate
    # make sure we get the first frame
    last_frame_time = -frame_interval
    with tqdm(total=total_frames, desc=video_file) as progress:
        while cap.isOpened():
            frame_exists, frame = cap.read()
            if frame_exists:
                # we don't need more than the framerate
                frame_ms = int(cap.get(cv2.CAP_PROP_POS_MSEC))
                if frame_ms - last_frame_time >= frame_interval:
                    last_frame_time = frame_ms
                    # resize the frame to fix in our target size
                    frame = resize_cover(frame, target_size)
                    # low quality to save space and bandwidth
                    encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), 50]
                    _, buffer = cv2.imencode(".jpg", frame, encode_param)
                    video_jpegs.append((frame_ms, buffer.tobytes()))
                progress.update(1)
            else:
                break
    cap.release()
    return video_jpegs


def get_video_hash(video_file):
    hash = hashlib.sha256(video_file.encode("utf-8")).hexdigest()
    return hash


def get_video_data(video_file):
    hash = get_video_hash(video_file)
    cache_file_name = f"cache/{hash}.pkl.gz"
    with gzip.open(cache_file_name, "rb") as f:
        data = pickle.load(f)
        audio = data["audio"]
        frames = data["frames"]
        return audio, frames


def process_video_file(
    video_path, video_file, target_size=(280, 240), sample_rate=16000, frame_rate=15
):
    hash = get_video_hash(video_file)
    cache_file_name = f"cache/{hash}.pkl.gz"
    # check if we've already processed this video
    if not os.path.exists(cache_file_name):
        # extract the audio and video frames
        full_path = os.path.join(video_path, video_file)
        audio = extract_audio(full_path)
        frames = extract_video_frames(full_path, target_size, frame_rate)
        # save the audio and frames to the cache
        with gzip.open(cache_file_name, "wb") as f:
            pickle.dump({"audio": audio, "frames": frames}, f)
    else:
        audio, frames = get_video_data(video_file)
    return audio, frames


def process_videos(
    video_path, target_size=(280, 240), sample_rate=16000, frame_rate=15
):
    # create the cache directory
    os.makedirs("cache", exist_ok=True)
    # get the list of files in the video path
    files = os.listdir(video_path)
    # filter out non-video files
    files = [f for f in files if f.endswith(".mp4")]
    # fort the files alphabetically
    files.sort()
    # process each video file
    video_data = []
    for file in tqdm(files, desc="Processing videos"):
        audio, frames = process_video_file(
            video_path, file, target_size, sample_rate, frame_rate
        )
        video_data.append((audio, frames))
    return video_data
