#pragma once
#include <freertos/FreeRTOS.h>
#include "AudioOutput.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "driver/sigmadelta.h"
#include "driver/timer.h"

/**
 * Base Class for both the ADC and I2S sampler
 **/
class PWMTimerOutput : public AudioOutput
{
private:
  gpio_num_t mPDMPin;
  uint32_t mSampleRate;
  SemaphoreHandle_t mBufferSemaphore;
  uint8_t *mBuffer=NULL;;
  int mCurrentIndex=0;
  int mBufferLength=0;
  uint8_t *mSecondBuffer=NULL;;
  int mSecondBufferLength=0;
  int mCount = 0;
  void onTimer();
public:
  PWMTimerOutput(gpio_num_t pdm_pin) : AudioOutput()
  {
    mPDMPin = pdm_pin;
    mBufferSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(mBufferSemaphore);
  }
  void write(uint8_t *samples, int count);
  void start(uint32_t sample_rate);
  void stop() {}

  friend void onTimerCallbackPWM(void *arg);
};
