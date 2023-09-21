#pragma once
#include <freertos/FreeRTOS.h>
#include <driver/i2s.h>

#include "AudioOutput.h"

/**
 * Base Class for both the ADC and I2S sampler
 **/
class DACOutput : public AudioOutput
{
public:
    DACOutput(i2s_port_t i2s_port) : AudioOutput(i2s_port) {}
    void start(uint32_t sample_rate);
    virtual int16_t process_sample(int16_t sample)
    {
        // DAC needs unsigned 16 bit samples
        // make them slightly quieter or they are really loud
        return sample + 32768;
    }
};
