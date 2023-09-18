#include <Arduino.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "VideoPlayer.h"
#include "audio_output/AudioOutput.h"

void _frameDownloaderTask(void *param)
{
  VideoPlayer *player = (VideoPlayer *)param;
  player->frameDownloaderTask();
}

void _framePlayerTask(void *param)
{
  VideoPlayer *player = (VideoPlayer *)param;
  player->framePlayerTask();
}

void _audioLoopTask(void *param)
{
  VideoPlayer *player = (VideoPlayer *)param;
  player->audioLoopTask();
}

VideoPlayer::VideoPlayer(const char *frameURL, const char *audioURL, TFT_eSPI &display, AudioOutput *audioOutput) : mFrameURL(frameURL), mAudioURL(audioURL), mDisplay(display), mState(VideoPlayerState::STOPPED), mAudioOutput(audioOutput)
{
}

void VideoPlayer::start()
{
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
  // launch the frame player task
  xTaskCreatePinnedToCore(
      _framePlayerTask,
      "Frame Player",
      10000,
      this,
      1,
      NULL,
      1);
  xTaskCreatePinnedToCore(_audioLoopTask, "audio_loop", 10000, this, 1, NULL, 0);
}

void VideoPlayer::setChannel(int channelIndex, int channelLength)
{
  mLastAudioTimeUpdateMs = millis();
  mLastAudioTimeUpdateMs = millis();
  mFrameReady = false;
  mAudioTimeMs = 0;
  mCurrentAudioSample = 0;
  mChannelIndex = channelIndex;
  mChannelLength = channelLength;
}

void VideoPlayer::play()
{
  if (mState == VideoPlayerState::PLAYING)
  {
    return;
  }
  mState = VideoPlayerState::PLAYING;
  mLastAudioTimeUpdateMs = millis();
  mAudioTimeMs = 0;
  mCurrentAudioSample = 0;
}

void VideoPlayer::stop()
{
  if (mState == VideoPlayerState::STOPPED)
  {
    return;
  }
  mState = VideoPlayerState::STOPPED;
  mLastAudioTimeUpdateMs = millis();
  mAudioTimeMs = 0;
  mCurrentAudioSample = 0;
  mDisplay.fillScreen(TFT_BLACK);
}

void VideoPlayer::pause()
{
  if (mState == VideoPlayerState::PAUSED)
  {
    return;
  }
  mState = VideoPlayerState::PAUSED;
}

void VideoPlayer::playStatic()
{
  if (mState == VideoPlayerState::STATIC)
  {
    return;
  }
  mState = VideoPlayerState::STATIC;
}

void VideoPlayer::frameDownloaderTask()
{
  char urlBuffer[200];
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
      vTaskDelay(100 / portTICK_PERIOD_MS);
      continue;
    }
    // work out the video time from a combination of the currentAudioSample and the elapsed time
    int elapsedTime = millis() - mLastAudioTimeUpdateMs;
    int videoTime = mAudioTimeMs + elapsedTime;
    if (WiFi.status() == WL_CONNECTED)
    {
      HTTPClient http;
      sprintf(urlBuffer, "%s/%d/%d", mFrameURL, mChannelIndex, videoTime);
      http.begin(urlBuffer);
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
        delay(1000);
      }
    }
    else
    {
      Serial.println("Not connected to WiFi");
      delay(1000);
    }
  }
}

// double buffer the dma drawing otherwise we get corruption
uint16_t *dmaBuffer[2] = {NULL, NULL};
int dmaBufferIndex = 0;
int _doDraw(JPEGDRAW *pDraw)
{
  VideoPlayer *player = (VideoPlayer *)pDraw->pUser;
  int numPixels = pDraw->iWidth * pDraw->iHeight;
  if (dmaBuffer[dmaBufferIndex] == NULL)
  {
    dmaBuffer[dmaBufferIndex] = (uint16_t *)malloc(numPixels * 2);
  }
  memcpy(dmaBuffer[dmaBufferIndex], pDraw->pPixels, numPixels * 2);
  TFT_eSPI &tft = player->mDisplay;
  tft.dmaWait();
  tft.setAddrWindow(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight);
  tft.pushPixels(dmaBuffer[dmaBufferIndex], numPixels);
  dmaBufferIndex = (dmaBufferIndex + 1) % 2;
  return 1;
}

static unsigned short x = 12345, y = 6789, z = 42, w = 1729;

unsigned short xorshift16()
{
  unsigned short t = x ^ (x << 5);
  x = y;
  y = z;
  z = w;
  w = w ^ (w >> 1) ^ t ^ (t >> 3);
  return w & 0xFFFF;
}

void VideoPlayer::framePlayerTask()
{
  uint16_t *staticBuffer = NULL;
  uint8_t *jpegBuffer = NULL;
  int jpegBufferLength = 0;
  int jpegLength = 0;
  while (true)
  {
    if (mState == VideoPlayerState::STOPPED || mState == VideoPlayerState::PAUSED)
    {
      // nothing to do - just wait
      vTaskDelay(100 / portTICK_PERIOD_MS);
      continue;
    }
    if (mState == VideoPlayerState::STATIC)
    {
      // draw random pixels to the screen to simulate static
      // we'll do this 8 rows of pixels at a time to save RAM
      int width = mDisplay.width();
      int height = 8;
      if (staticBuffer == NULL)
      {
        staticBuffer = (uint16_t *)malloc(width * height * 2);
      }
      for (int i = 0; i < mDisplay.height(); i++)
      {
        for (int p = 0; p < width * height; p++)
        {
          int grey = xorshift16() >> 8;
          staticBuffer[p] = mDisplay.color565(grey, grey, grey);
        }
        // mDisplay.dmaWait();
        mDisplay.dmaWait();
        mDisplay.setAddrWindow(0, i * height, width, height);
        mDisplay.pushPixelsDMA(staticBuffer, width * height);
      }
      vTaskDelay(50 / portTICK_PERIOD_MS);
      continue;
    }
    // lock the current frame buffer
    xSemaphoreTake(mCurrentFrameMutex, portMAX_DELAY);
    if (!mFrameReady)
    {
      xSemaphoreGive(mCurrentFrameMutex);
      vTaskDelay(100 / portTICK_PERIOD_MS);
      continue;
    }
    // grab a copy of the current frame buffer
    if (mCurrentFrameBufferLength > jpegBufferLength)
    {
      jpegBuffer = (uint8_t *)realloc(jpegBuffer, mCurrentFrameBufferLength);
      jpegBufferLength = mCurrentFrameBufferLength;
    }
    memcpy(jpegBuffer, mCurrentFrameBuffer, mCurrentFrameLength);
    jpegLength = mCurrentFrameLength;
    // unlock the frame buffer so the downloader can carry on
    xSemaphoreGive(mCurrentFrameMutex);
    // make sure the downloader gets the next frame
    mFrameReady = false;
    // do the actual drawing
    mDisplay.startWrite();
    if (mJpeg.openRAM(jpegBuffer, jpegLength, _doDraw))
    {
      mJpeg.setUserPointer(this);
      mJpeg.setPixelType(RGB565_BIG_ENDIAN);
      mJpeg.decode(0, 0, 0);
      mJpeg.close();
    }
    if (mAudioTimeMs < 2000) {
      mDisplay.setCursor(20, 20);
      mDisplay.setTextColor(TFT_GREEN, TFT_BLACK);
      mDisplay.printf("%d", mChannelIndex);
    }
    mDisplay.endWrite();
  }
}

void VideoPlayer::audioLoopTask()
{
  char urlBuffer[200];
  int8_t *audioData = (int8_t *)malloc(16000);
  while (true)
  {
    if (mState != VideoPlayerState::PLAYING)
    {
      // nothing to do - just wait
      vTaskDelay(100 / portTICK_PERIOD_MS);
      continue;
    }
    // download the audio data
    if (WiFi.status() == WL_CONNECTED)
    {
      HTTPClient http;
      sprintf(urlBuffer, "%s/%d/%d/16000", mAudioURL, mChannelIndex, mCurrentAudioSample);
      http.begin(urlBuffer);
      int httpCode = http.GET();
      if (httpCode == HTTP_CODE_OK)
      {
        // read the audio data into the buffer
        int audioLength = http.getSize();
        http.getStreamPtr()->readBytes((uint8_t *)audioData, audioLength);
        // play the audio
        for(int i=0; i<audioLength; i+=1000) {
          mAudioOutput->write(audioData + i, min(1000, audioLength - i));
          mCurrentAudioSample += min(1000, audioLength - i);
          mLastAudioTimeUpdateMs = millis();
          if (mCurrentAudioSample > mChannelLength || mState != VideoPlayerState::PLAYING)
          {
            mCurrentAudioSample = 0;
            mLastAudioTimeUpdateMs = millis();
            mAudioTimeMs = 1000 * mCurrentAudioSample / 16000;
            break;
          }
          mLastAudioTimeUpdateMs = millis();
          mAudioTimeMs = 1000 * mCurrentAudioSample / 16000;
        }
      }
      else
      {
        Serial.printf("HTTP error: %d\n", httpCode);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
      }
    }
    else
    {
      Serial.println("Not connected to WiFi");
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
}