
#include <esp_log.h>
#include "I2SOutput.h"

const size_t NUM_FRAMES_TO_SEND=1024;
const char *TAG = "I2SOutput";


I2SOutput::I2SOutput(i2s_port_t i2s_port, i2s_pin_config_t &i2s_pins) : m_i2s_port(i2s_port), m_i2s_pins(i2s_pins)
{
}

void I2SOutput::start(uint32_t sample_rate)
{
  // space to store franes that we are sending to the I2S device
  // 2 channels, 16 bits per sample, NUM_FRAMES_TO_SEND samples
  m_tmp_frames = (int16_t *)malloc(2 * sizeof(int16_t) * NUM_FRAMES_TO_SEND);
  // i2s config for writing both channels of I2S
  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
      .sample_rate = sample_rate,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S),
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 4,
      .dma_buf_len = 1024,
      .use_apll = false,
      .tx_desc_auto_clear = true,
      .fixed_mclk = 0};
  // install and start i2s driver
  i2s_driver_install(m_i2s_port, &i2s_config, 0, NULL);
  // set up the i2s pins
  i2s_set_pin(m_i2s_port, &m_i2s_pins);
  // clear the DMA buffers
  i2s_zero_dma_buffer(m_i2s_port);
  // kick things off
  i2s_start(m_i2s_port);
}

void I2SOutput::stop()
{
  i2s_stop(m_i2s_port);
  i2s_driver_uninstall(m_i2s_port);
  free(m_tmp_frames);
  m_tmp_frames = NULL; 
}

template<class SampleT> void I2SOutput::write(SampleT *samples, int count, int shift)
{
  int sample_index = 0;
  while (sample_index < count)
  {
    int samples_to_send = 0;
    for (int i = 0; i < NUM_FRAMES_TO_SEND && sample_index < count; i++)
    {
      int sample = samples[sample_index] * volume / 10;
      m_tmp_frames[i * 2] = sample << shift;
      m_tmp_frames[i * 2 + 1] = sample << shift;
      samples_to_send++;
      sample_index++;
    }
    // write data to the i2s peripheral
    size_t bytes_written = 0;
    i2s_write(m_i2s_port, m_tmp_frames, samples_to_send * sizeof(int16_t) * 2, &bytes_written, portMAX_DELAY);
    if (bytes_written != samples_to_send * sizeof(int16_t) * 2)
    {
      ESP_LOGE(TAG, "Did not write all bytes");
    }
  }
}

void I2SOutput::write(int8_t *samples, int count)
{
  write(samples, count, 8);
}
