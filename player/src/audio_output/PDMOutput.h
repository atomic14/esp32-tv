#pragma once
#include <freertos/FreeRTOS.h>
#include <driver/i2s.h>

#include "I2SBase.h"

/**
 * Base Class for both the ADC and I2S sampler
 **/
class PDMOutput : public I2SBase
{
private:
    i2s_pin_config_t m_i2s_pins;
public:
    PDMOutput(i2s_port_t i2s_port, i2s_pin_config_t &i2s_pins);
    void start(uint32_t sample_rate);
    int16_t process_sample(int16_t sample) { 
        float normalised = (float)sample / 32768.0f;
        // give it some welly
        normalised *= 4.0f;
        // now clamp it to 1 or -1 using a nice clipping function
        normalised = (normalised / (1.0 + 0.28 * (normalised * normalised)));
        return normalised * 32768.0f;
    }
};
