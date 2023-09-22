#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include "VideoPlayerState.h"

class AudioSource {
  public:
    virtual void start() {};
    // get up to maxSamples of audio data and return the number of samples retrieved
    virtual int getAudioSamples(int8_t **buffer, size_t &bufferSize, int currentAudioSample) = 0;
};