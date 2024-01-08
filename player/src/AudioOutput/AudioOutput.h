#pragma once

#include <freertos/FreeRTOS.h>
#include <driver/i2s.h>

/**
 * Base Class for both the DAC and I2S output
 **/
class AudioOutput
{
protected:
  int mVolume = 10;
public:
  AudioOutput();
  virtual void start(uint32_t sample_rate) = 0;
  virtual void stop() = 0;
  // override this in derived classes to turn the sample into
  // something the output device expects - for the default case
  // this is simply a pass through
  virtual int16_t process_sample(int16_t sample) { return sample; }
  virtual void write(int8_t *samples, int count) = 0;

  void setVolume(int volume){
    if (volume > 10 || volume < 0) mVolume = 10;
    else mVolume = volume;
  }

  void volumeUp() {
    if (mVolume == 10) {
      return;
    }
    mVolume++;
  }
  void volumeDown() {
    if (mVolume == 0) {
      return;
    }
    mVolume--;
  }
};
