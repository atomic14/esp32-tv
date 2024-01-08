#pragma once

#include <freertos/FreeRTOS.h>
#include <driver/i2s.h>

#include "AudioOutput.h"

/**
 * Base Class for both the DAC and I2S output
 **/
class I2SBase : public AudioOutput
{
protected:
  i2s_port_t m_i2s_port = I2S_NUM_0;
  int16_t *m_tmp_frames = NULL;
public:
  I2SBase(i2s_port_t i2s_port);
  void stop();
  void write(int8_t *samples, int count);
  // override this in derived classes to turn the sample into
  // something the output device expects - for the default case
  // this is simply a pass through
  virtual int16_t process_sample(int16_t sample) { return sample; }
};
