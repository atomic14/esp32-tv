#pragma once
#include "JPEGDEC.h"

class TFT_eSPI;
class I2SOutput;

enum class VideoPlayerState {
  STOPPED,
  PLAYING,
  PAUSED,
  STATIC
};

class VideoPlayer {
  private:
    int mChannelIndex = -1;
    int mChannelLength = 0; // TODO - this should be read from the channel data
    VideoPlayerState mState = VideoPlayerState::STOPPED;

    // video playing
    TFT_eSPI &mDisplay;
    JPEGDEC mJpeg;
    const char *mFrameURL = NULL;
    SemaphoreHandle_t mCurrentFrameMutex = NULL;
    // the current frame we want to display
    uint8_t *mCurrentFrameBuffer = NULL;
    // the length of the current frame buffer
    size_t mCurrentFrameBufferLength = 0;
    // the length of the current frame (may be shorter than the frame buffer)
    size_t mCurrentFrameLength = 0;
    // is there a frame ready for us to display?
    bool mFrameReady = false;
    // the time reference from the audio player - starts from 0 when the audio starts
    int mAudioTimeMs = 0;
    // the time when we last got an audio time update - used to calculate elapsed time
    int mLastAudioTimeUpdateMs = 0;

    // audio playing
    const char *mAudioURL = NULL;
    int mCurrentAudioSample = 0;
    I2SOutput *mAudioOutput = NULL;

    friend void _frameDownloaderTask(void *param);
    friend void _framePlayerTask(void *param);
    friend void _audioLoopTask(void *param);

    void frameDownloaderTask();
    void framePlayerTask();
    void audioLoopTask();

    friend int _doDraw(JPEGDRAW *pDraw);

  public:
    VideoPlayer(const char *frameURL, const char *audioURL, TFT_eSPI &display, I2SOutput *audioOutput);
    void setChannel(int channelIndex, int channelLength);
    void start();
    void play();
    void stop();
    void pause();
    void playStatic();
    // we use the audio time as our reference for the video time
    void setAudioTimeMs(int ms);
};