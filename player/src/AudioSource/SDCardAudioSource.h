#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "AudioSource.h"

class SDCardChannelData;

class SDCardAudioSource : public AudioSource
{
private:
  SDCardChannelData *mChannelData = NULL;

public:
  SDCardAudioSource(SDCardChannelData *channelData);
  int getAudioSamples(int8_t **buffer, size_t &bufferSize, int currentAudioSample);
};