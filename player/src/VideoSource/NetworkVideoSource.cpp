#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "NetworkVideoSource.h"
#include "../ChannelData/NetworkChannelData.h"

void NetworkVideoSource::_frameDownloaderTask(void *param)
{
  NetworkVideoSource *networkVideoSource = (NetworkVideoSource *)param;
  networkVideoSource->frameDownloaderTask();
}

void NetworkVideoSource::frameDownloaderTask()
{
  HTTPClient http;
  http.setReuse(true);
  uint8_t *downloadBuffer = NULL;
  int downloadBufferLength = 0;
  while (true)
  {
    if (mState == VideoPlayerState::STOPPED || mState == VideoPlayerState::STATIC)
    {
      vTaskDelay(100 / portTICK_PERIOD_MS);
      continue;
    }
    if (mState == VideoPlayerState::PAUSED)
    {
      // video time is not passing, so keep moving the start time forward so it is now
      mLastAudioTimeUpdateMs = millis();
      vTaskDelay(100 / portTICK_PERIOD_MS);
      continue;
    }
    // do we need to download a frame?
    if (mFrameReady)
    {
      // we already have a frame ready, so just wait
      vTaskDelay(10 / portTICK_PERIOD_MS);
      continue;
    }
    // work out the video time from a combination of the currentAudioSample and the elapsed time
    int elapsedTime = millis() - mLastAudioTimeUpdateMs;
    int videoTime = mAudioTimeMs + elapsedTime;
    if (WiFi.status() == WL_CONNECTED)
    {
      std::string url = mChannelData->getFrameURL() + "/" + std::to_string(videoTime);
      http.begin(url.c_str());
      int httpCode = http.GET();

      if (httpCode == HTTP_CODE_OK)
      {
        // read the image into our local buffer
        int jpegLength = http.getSize();
        if (jpegLength > downloadBufferLength)
        {
          downloadBuffer = (uint8_t *)realloc(downloadBuffer, jpegLength);
          downloadBufferLength = jpegLength;
        }
        http.getStreamPtr()->readBytes(downloadBuffer, jpegLength);
        // lock the image buffer
        xSemaphoreTake(mCurrentFrameMutex, portMAX_DELAY);
        // reallocate the image buffer if necessary
        if (jpegLength > mCurrentFrameBufferLength)
        {
          mCurrentFrameBuffer = (uint8_t *)realloc(mCurrentFrameBuffer, jpegLength);
          mCurrentFrameBufferLength = jpegLength;
        }
        // copy the image buffer
        memcpy(mCurrentFrameBuffer, downloadBuffer, jpegLength);
        mCurrentFrameLength = jpegLength;
        // don't set this flag if we aren't playing otherwise we might trigger a draw
        if (mState == VideoPlayerState::PLAYING)
        {
          mFrameReady = true;
        }
        // unlock the image buffer
        xSemaphoreGive(mCurrentFrameMutex);
        // Serial.printf("Read %d bytes in %d ms\n", download_image_length, millis() - start_download_time);
      }
      else
      {
        Serial.printf("HTTP error: %d\n", httpCode);
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }
    }
    else
    {
      Serial.println("Not connected to WiFi");
      delay(1000);
    }
  }
}


NetworkVideoSource::NetworkVideoSource(NetworkChannelData *channelData) : mChannelData(channelData)
{
}

void NetworkVideoSource::start() {
  // create a mutex to control access to the JPEG buffer
  mCurrentFrameMutex = xSemaphoreCreateMutex();
  // launch the frame downloader task
  xTaskCreatePinnedToCore(
      _frameDownloaderTask,
      "Frame Downloader",
      10000,
      this,
      1,
      NULL,
      0);
}

bool NetworkVideoSource::getVideoFrame(uint8_t **buffer, size_t &bufferLength, size_t &frameLength) {
  if(mCurrentFrameBuffer == NULL) {
    return false;
  }
  bool copiedFrame = false;
  // lock the image buffer
  xSemaphoreTake(mCurrentFrameMutex, portMAX_DELAY);
  // if the frame is ready, copy it to the buffer
  if (mFrameReady) {
    mFrameReady=false;
    copiedFrame = true;
    // reallocate the image buffer if necessary
    if (mCurrentFrameBufferLength > bufferLength) {
      *buffer = (uint8_t *)realloc(*buffer, mCurrentFrameBufferLength);
      bufferLength = mCurrentFrameBufferLength;
    }
    // copy the image buffer
    memcpy(*buffer, mCurrentFrameBuffer, mCurrentFrameLength);
    frameLength = mCurrentFrameLength;
  }
  // unlock the image buffer
  xSemaphoreGive(mCurrentFrameMutex);
  // return true if we copied a frame, false otherwise
  return copiedFrame;
}

void NetworkVideoSource::setChannel(int channel) {
  mLastAudioTimeUpdateMs = millis();
  mFrameReady = false;
  mAudioTimeMs = 0;
}
