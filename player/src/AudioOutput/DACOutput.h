#pragma once
#include <freertos/FreeRTOS.h>
#include <driver/i2s.h>

#include "I2SBase.h"

/**
 * Base Class for both the ADC and I2S sampler
 **/
class DACOutput : public I2SBase
{
public:
    DACOutput(i2s_port_t i2s_port) : I2SBase(i2s_port) {}
    void start(uint32_t sample_rate);
    virtual int16_t process_sample(int16_t sample)
    {
        // DAC needs unsigned 16 bit samples
        return sample + 32768;
    }
};
