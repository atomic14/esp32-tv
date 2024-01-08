#include "PDMTimerOutput.h"
#include "freertos/semphr.h"
#include "driver/sigmadelta.h"
#include <string.h>
#include <Arduino.h>


IRAM_ATTR void onTimerCallback(void *param)
{
  PDMTimerOutput *output = (PDMTimerOutput *)param;
  timer_spinlock_take(TIMER_GROUP_0);
  timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_0);
  timer_group_enable_alarm_in_isr(TIMER_GROUP_0, TIMER_0);
  timer_spinlock_give(TIMER_GROUP_0);
  output->onTimer();
}

void PDMTimerOutput::start(uint32_t sample_rate)
{
  mSampleRate = sample_rate;

  sigmadelta_config_t config;
  config.channel = SIGMADELTA_CHANNEL_0;
  config.sigmadelta_duty = 0;
  config.sigmadelta_prescale = 255;
  config.sigmadelta_gpio = mPDMPin;

  // configure the PDM
  esp_err_t result = sigmadelta_config(&config);
  if (result != ESP_OK)
  {
    Serial.printf("Error initializing PDM: %d\n", result);
  }
  // result = sigmdelta_begin(SIGMADELTA_CHANNEL_0);
  // if (result != ESP_OK)
  // {
  //   Serial.printf("Error beginning PDM: %d\n", result);
  // }
  // pinMode(18, OUTPUT);

  // create a timer that will fire at the sample rate
  timer_config_t timer_config = {
      .alarm_en = TIMER_ALARM_EN,
      .counter_en = TIMER_PAUSE,
      .intr_type = TIMER_INTR_LEVEL,
      .counter_dir = TIMER_COUNT_UP,
      .auto_reload = TIMER_AUTORELOAD_EN,
      .divider = 80};
  result = timer_init(TIMER_GROUP_0, TIMER_0, &timer_config);
  if (result != ESP_OK)
  {
    Serial.printf("Error initializing timer: %d\n", result);
  }
  result = timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0x00000000ULL);
  if (result != ESP_OK)
  {
    Serial.printf("Error setting timer counter value: %d\n", result);
  }
  result = timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, 1000000 / mSampleRate);
  if (result != ESP_OK)
  {
    Serial.printf("Error setting timer alarm value: %d\n", result);
  }
  // timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, 1000);
  result = timer_enable_intr(TIMER_GROUP_0, TIMER_0);
  if (result != ESP_OK)
  {
    Serial.printf("Error enabling timer interrupt: %d\n", result);
  }
  result = timer_isr_register(TIMER_GROUP_0, TIMER_0, &onTimerCallback, this, ESP_INTR_FLAG_IRAM, NULL);
  if (result != ESP_OK)
  {
    Serial.printf("Error registering timer interrupt: %d\n", result);
    // print the string error
    const char *err = esp_err_to_name(result);
    Serial.printf("Error: %s\n", err);
  }
  result = timer_start(TIMER_GROUP_0, TIMER_0);
  if (result != ESP_OK)
  {
    Serial.printf("Error starting timer: %d\n", result);
  }
  Serial.println("PDM Started");
}

void PDMTimerOutput::write(int8_t *samples, int count)
{
  // Serial.printf("Count %d\n", mCount);
  while (true)
  {
    if(xSemaphoreTake(mBufferSemaphore, portMAX_DELAY)) {
      // is the second buffer empty?
      if (mSecondBufferLength == 0)
      {
        //Serial.println("Filling second buffer");
        // make sure there's enough room for the samples
        mSecondBuffer = (int8_t *)realloc(mSecondBuffer, count);
        // copy them into the second buffer
        memcpy(mSecondBuffer, samples, count);
        // second buffer is now full of samples
        mSecondBufferLength = count;
        // unlock the mutext and return
        xSemaphoreGive(mBufferSemaphore);
        return;
      }
      // no room in the second buffer so wait for the first buffer to be emptied
      xSemaphoreGive(mBufferSemaphore);
    }
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void PDMTimerOutput::onTimer()
{
  // output a sample from the buffer if we have one
  if (mCurrentIndex < mBufferLength)
  {
    mCount++;
    // get the first sample from the buffer
    int16_t sample = mBuffer[mCurrentIndex];
    mCurrentIndex++;
    sample *= 20;
    if (sample > 90) {
      sample = 90;
    }
    if (sample < -90) {
      sample = -90;
    }
    // write the sample to the PDM
    sigmadelta_set_duty(SIGMADELTA_CHANNEL_0, sample);
  }
  if(mCurrentIndex >= mBufferLength)
  {
    // do we have any data in teh second buffer?
    BaseType_t xHigherPriorityTaskWoken;
    if (xSemaphoreTakeFromISR(mBufferSemaphore, &xHigherPriorityTaskWoken) == pdTRUE)
    {
      if (mSecondBufferLength > 0) {
        // swap the buffers
        int8_t *tmp = mBuffer;
        mBuffer = mSecondBuffer;
        mBufferLength = mSecondBufferLength;
        mSecondBuffer = tmp;
        mSecondBufferLength = 0;
        mCurrentIndex = 0;
      }
      xSemaphoreGiveFromISR(mBufferSemaphore, &xHigherPriorityTaskWoken);
    }
    if(xHigherPriorityTaskWoken) {
      portYIELD_FROM_ISR();
    }
  }
}