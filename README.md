# ESP32 Video Streaming!

Yes - it actually works! Streaming video with audio to an ESP32!

[![Demo Video](https://img.youtube.com/vi/G6MROvlLeKE/0.jpg)](https://www.youtube.com/watch?v=G6MROvlLeKE)

You can see a video of it in action [here](https://www.youtube.com/watch?v=G6MROvlLeKE).


There's two projects in this repo - one for the ESP32 firmware and another for the server.

The README files in each project have more details.

# How Does It Work?

The server is pretty simple, it has a few endpoints:

- `/channel_info` - returns a list of channel lengths in audio samples
- `/frame/<int:channel_index>/<int:ms>` - returns a JPEG image for the given channel at the given time (in ms)
- `/audio/<int:channel_index>/<int:start>/<int:length>` - returns 8 bit PCM audio at 16KHz for the given channel starting from the given sample index and for the given length (in samples)

The ESP32 firmware connects to the server and requests the channel info. The video playback is locked to the audio sample playback. The audio is played back using the I2S peripheral and we use that to know how much time has elapsed to request the correct frames. This way the video and audio are always in sync.

You can get around 15 frames per second at 280x240 resolution, the main limitation is WiFi bandwidth and decoding the JPEGs.
