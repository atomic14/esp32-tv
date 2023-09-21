#include <Arduino.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "VideoPlayer.h"
#include "audio_output/AudioOutput.h"
#include "VideoSource/VideoSource.h"
#include "AudioSource/AudioSource.h"

void VideoPlayer::_framePlayerTask(void *param)
{
  VideoPlayer *player = (VideoPlayer *)param;
  player->framePlayerTask();
}

void VideoPlayer::_audioPlayerTask(void *param)
{
  VideoPlayer *player = (VideoPlayer *)param;
  player->audioPlayerTask();
}

VideoPlayer::VideoPlayer(VideoSource *videoSource, AudioSource *audioSource, TFT_eSPI &display, AudioOutput *audioOutput)
: mVideoSource(videoSource), mAudioSource(audioSource), mDisplay(display), mState(VideoPlayerState::STOPPED), mAudioOutput(audioOutput)
{
}

void VideoPlayer::start()
{
  mVideoSource->start();
  mAudioSource->start();
  // launch the frame player task
  xTaskCreatePinnedToCore(
      _framePlayerTask,
      "Frame Player",
      10000,
      this,
      1,
      NULL,
      1);
  xTaskCreatePinnedToCore(_audioPlayerTask, "audio_loop", 10000, this, 1, NULL, 1);
}

void VideoPlayer::setChannel(int channel, int channelLength)
{
  mVideoSource->setChannel(channel);
  mCurrentAudioSample = 0;
  mChannel = channel;
  mChannelLength = channelLength;
  mChannelVisible = millis();
}

void VideoPlayer::play()
{
  if (mState == VideoPlayerState::PLAYING)
  {
    return;
  }
  mState = VideoPlayerState::PLAYING;
  mVideoSource->setState(VideoPlayerState::PLAYING);
  mCurrentAudioSample = 0;
}

void VideoPlayer::stop()
{
  if (mState == VideoPlayerState::STOPPED)
  {
    return;
  }
  mState = VideoPlayerState::STOPPED;
  mVideoSource->setState(VideoPlayerState::STOPPED);
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
  mVideoSource->setState(VideoPlayerState::PAUSED);
}

void VideoPlayer::playStatic()
{
  if (mState == VideoPlayerState::STATIC)
  {
    return;
  }
  mState = VideoPlayerState::STATIC;
  mVideoSource->setState(VideoPlayerState::STATIC);
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
  #ifdef USE_DMA
  tft.dmaWait();
  #endif
  tft.setAddrWindow(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight);
  #ifdef USE_DMA
  tft.pushPixelsDMA(dmaBuffer[dmaBufferIndex], numPixels);
  #else
  tft.pushPixels(dmaBuffer[dmaBufferIndex], numPixels);
  #endif
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
  size_t jpegBufferLength = 0;
  size_t jpegLength = 0;
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
        #ifdef USE_DMA
        mDisplay.dmaWait();
        #endif
        mDisplay.setAddrWindow(0, i * height, width, height);
        #ifdef USE_DMA
        mDisplay.pushPixelsDMA(staticBuffer, width * height);
        #else
        mDisplay.pushPixels(staticBuffer, width * height);
        #endif
      }
      vTaskDelay(50 / portTICK_PERIOD_MS);
      continue;
    }
    // get the next frame
    if (!mVideoSource->getVideoFrame(&jpegBuffer, jpegBufferLength, jpegLength))
    {
      // no frame ready yet
      vTaskDelay(10 / portTICK_PERIOD_MS);
      continue;
    }
    mDisplay.startWrite();
    if (mJpeg.openRAM(jpegBuffer, jpegLength, _doDraw))
    {
      mJpeg.setUserPointer(this);
      mJpeg.setPixelType(RGB565_BIG_ENDIAN);
      mJpeg.decode(0, 0, 0);
      mJpeg.close();
    }
    // show channel indicator 
    if (millis() - mChannelVisible < 2000) {
      mDisplay.setCursor(20, 20);
      mDisplay.setTextColor(TFT_GREEN, TFT_BLACK);
      mDisplay.printf("%d", mChannel);
    }
    mDisplay.endWrite();
  }
}

void VideoPlayer::audioPlayerTask()
{
  int8_t *audioData = (int8_t *)malloc(16000);
  while (true)
  {
    if (mState != VideoPlayerState::PLAYING)
    {
      // nothing to do - just wait
      vTaskDelay(100 / portTICK_PERIOD_MS);
      continue;
    }
    // get audio data to play
    int audioLength = mAudioSource->getAudioSamples(audioData, 16000, mChannel, mCurrentAudioSample);
    if (audioLength > 0) {
      // play the audio
      for(int i=0; i<audioLength; i+=1000) {
        mAudioOutput->write(audioData + i, min(1000, audioLength - i));
        mCurrentAudioSample += min(1000, audioLength - i);
        if (mCurrentAudioSample > mChannelLength || mState != VideoPlayerState::PLAYING)
        {
          mCurrentAudioSample = 0;
          mVideoSource->updateAudioTime(1000 * mCurrentAudioSample / 16000);
          break;
        }
        mVideoSource->updateAudioTime(1000 * mCurrentAudioSample / 16000);
      }
    }
    else
    {
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }
  }
}