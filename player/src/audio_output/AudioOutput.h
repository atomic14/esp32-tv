#pragma once

#include <freertos/FreeRTOS.h>
#include <driver/i2s.h>

/**
 * Base Class for both the DAC and I2S output
 **/
class AudioOutput
{
private:
  template<class SampleT> void write(SampleT *samples, int count, int shift=0);
protected:
  i2s_port_t m_i2s_port = I2S_NUM_0;
  int16_t *m_tmp_frames = NULL;
  int mVolume = 10;
public:
  AudioOutput(i2s_port_t i2s_port);
  virtual void start(uint32_t sample_rate) = 0;
  void stop();
  // override this in derived classes to turn the sample into
  // something the output device expects - for the default case
  // this is simply a pass through
  virtual int16_t process_sample(int16_t sample) { return sample; }
  // specialisation of the template function to avoid having to include the implementation in the header
  void write(int8_t *samples, int count);
  void write(int16_t *samples, int count);

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
