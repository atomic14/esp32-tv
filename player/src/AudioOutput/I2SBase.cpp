
#include <Arduino.h>
#include "I2SBase.h"
#include <esp_log.h>
#include <driver/i2s.h>

static const char *TAG = "AUDIO";

// number of frames to try and send at once (a frame is a left and right sample)
const size_t NUM_FRAMES_TO_SEND=1024;

I2SBase::I2SBase(i2s_port_t i2s_port) : m_i2s_port(i2s_port)
{
  m_tmp_frames = (int16_t *)malloc(2 * sizeof(int16_t) * NUM_FRAMES_TO_SEND);
}

void I2SBase::stop()
{
  // stop the i2S driver
  i2s_stop(m_i2s_port);
  i2s_driver_uninstall(m_i2s_port);
}

void I2SBase::write(int8_t *samples, int count)
{
  int sample_index = 0;
  while (sample_index < count)
  {
    int samples_to_send = 0;
    for (int i = 0; i < NUM_FRAMES_TO_SEND && sample_index < count; i++)
    {
      // shift up to 16 bit samples
      int sample = process_sample((samples[sample_index] * mVolume / 10) << 8);
      m_tmp_frames[i * 2] = sample;
      m_tmp_frames[i * 2 + 1] = sample;
      samples_to_send++;
      sample_index++;
    }
    // write data to the i2s peripheral
    size_t bytes_written = 0;
    esp_err_t res = i2s_write(m_i2s_port, m_tmp_frames, samples_to_send * sizeof(int16_t) * 2, &bytes_written, 1000 / portTICK_PERIOD_MS);
    if (res != ESP_OK)
    {
      ESP_LOGE(TAG, "Error sending audio data: %d", res);
    }
    if (bytes_written != samples_to_send * sizeof(int16_t) * 2)
    {
      ESP_LOGE(TAG, "Did not write all bytes");
    }
  }
}
