# ESP32 Movie Streaming Firmware

Almost all the configuration is done in the `platformio.ini` file. You can use the existing settings as the basis for new boards.

At a minimum you'll need a screen that works with the TFT_eSPI library. I'm using this one in my project - https://s.click.aliexpress.com/e/_DmStZcn

You'll also need some way of getting sound out. I recommend the MAX98357A breakout board - https://s.click.aliexpress.com/e/_DDnBold shop around as there are plenty of sellers for this.

The code should work with pretty much any ESP32 board, but I've only tested it on a few.

There is currently a bug in the I2S code when using DAC for audio output - you'll need to use an older version of the Arduino ESP32 Core - check the Cheap Yellow Display settings in `platformio.ini` for the correct version.