#pragma once

#include <freertos/FreeRTOS.h>
#include <driver/i2s.h>


class I2SOutput
{
private:
    i2s_pin_config_t m_i2s_pins;
    i2s_port_t m_i2s_port = I2S_NUM_0;
    int16_t *m_tmp_frames = NULL;
    int volume = 10;
public:
    I2SOutput(i2s_port_t i2s_port, i2s_pin_config_t &i2s_pins);
    void start(uint32_t sample_rate);
    void stop();
    template<class SampleT> void write(SampleT *samples, int count, int shift=0);
    void write(int8_t *samples, int count);

    void volumeUp() {
        if (volume == 10) {
            return;
        }
        volume++;
    }
    void volumeDown() {
        if (volume == 0) {
            return;
        }
        volume--;
    }
};
