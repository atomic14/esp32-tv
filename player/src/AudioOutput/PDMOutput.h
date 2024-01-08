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
};
