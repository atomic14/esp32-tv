
#include "DACOutput.h"
#include <Arduino.h>

void DACOutput::start(uint32_t sample_rate)
{
    // only include this if we're using DAC - the ESP32-S3 will fail compilation if we include this
    #ifdef USE_DAC_AUDIO
    // i2s config for writing both channels of I2S
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
        .sample_rate = sample_rate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_MSB,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 1024,
        .use_apll = true,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0};
    //install and start i2s driver
    esp_err_t res = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    if (res != ESP_OK) {
        Serial.printf("i2s_driver_install failed: %d\n", res);
    }
    // enable the DAC channels
    res = i2s_set_dac_mode(I2S_DAC_CHANNEL_BOTH_EN);
    if (res != ESP_OK) {
        Serial.printf("i2s_set_dac_mode failed: %d\n", res);
    }
    // clear the DMA buffers
    res = i2s_zero_dma_buffer(I2S_NUM_0);
    if (res != ESP_OK) {
        Serial.printf("i2s_zero_dma_buffer failed: %d\n", res);
    }

    res = i2s_start(I2S_NUM_0);
    if (res != ESP_OK) {
        Serial.printf("i2s_start failed: %d\n", res);
    }
    #endif
}
