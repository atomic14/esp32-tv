#pragma once
#include "JPEGDEC.h"

#include "VideoPlayerState.h"

class TFT_eSPI;
class AudioOutput;

class VideoSource;
class AudioSource;
class ChannelData;

class VideoPlayer {
  private:
    int mChannelVisible = 0;
    VideoPlayerState mState = VideoPlayerState::STOPPED;

    // video playing
    TFT_eSPI &mDisplay;
    JPEGDEC mJpeg;

    // video source
    VideoSource *mVideoSource = NULL;
    // audio source
    AudioSource *mAudioSource = NULL;
    // channel information
    ChannelData *mChannelData = NULL;

    // audio playing
    int mCurrentAudioSample = 0;
    AudioOutput *mAudioOutput = NULL;

    static void _framePlayerTask(void *param);
    static void _audioPlayerTask(void *param);

    void framePlayerTask();
    void audioPlayerTask();

    friend int _doDraw(JPEGDRAW *pDraw);

  public:
    VideoPlayer(ChannelData *channelData, VideoSource *videoSource, AudioSource *audioSource, TFT_eSPI &display, AudioOutput *audioOutput);
    void setChannel(int channelIndex);
    void start();
    void play();
    void stop();
    void pause();
    void playStatic();
};