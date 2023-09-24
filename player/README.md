# ESP32 Movie Streaming Firmware

Almost all the configuration is done in the `platformio.ini` file. You can use the existing settings as the basis for new boards.

At a minimum you'll need a screen that works with the TFT_eSPI library. I'm using this one in my project - https://s.click.aliexpress.com/e/_DmStZcn

You'll also need some way of getting sound out. I recommend the MAX98357A breakout board - https://s.click.aliexpress.com/e/_DDnBold shop around as there are plenty of sellers for this.

The code should work with pretty much any ESP32 board, but I've only tested it on a few.

## Suppoer for AVI files on an SDCard

This is a work in progress, but it should be able to stream AVI files from an SDCard - see the cheap yellow display setup in the ini file for the config.

Enabled this by adding the following defines to platformio.ini:

```
-DUSE_SDCARD
-DSD_CARD_MISO=GPIO_NUM_3
-DSD_CARD_MOSI=GPIO_NUM_8
-DSD_CARD_CLK=GPIO_NUM_46
-DSD_CARD_CS=GPIO_NUM_18
```

Modify the pin numbers as needed.

This will turn off WiFi streaming and stream AVI files from the SD Card.

To create a compatible AVI file, you can use ffmpeg:

```
ffmpeg -i input.mp4 -vf "scale=320:240" -r 15 -c:v mjpeg -q:v 10 -acodec pcm_u8 -af "loudnorm" -ar 16000 -ac 1 output.avi
```

* -i input.mp4: Specifies the input file named input.mp4.
* -vf "scale=320:240": Sets the video filter to scale the video to 320x240 resolution - this matches the CYD
* -r 15: Sets the frame rate to 15fps.
* -c:v mjpeg: Sets the video codec to MJPEG.
* -q:v 10: Sets the video quality (lower values mean higher quality; you can adjust this as needed) - range 2-31
* -acodec pcm_u8: Sets the audio codec to 8-bit PCM - NOTE - AVI files cannot contain pcm_s8 audio, so there's some extra processing in the audio pipeline to handle this.
* -ar 16000: Sets the audio sample rate to 16KHz.
* -ac 1: Sets the audio to mono (single channel).
* output.avi: Specifies the output file named output.avi.

If you want to get fancy you can use the loudnorm filter to normalize the audio levels. There's a script in the `tools` folder that will do this for you. You will need `jq` installed to use it.

```
./tools/convert_movie_with_nornalization.sh input.mp4 output.avi
```

