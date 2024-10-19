#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "AudioSource.h"

class NetworkChannelData;

class NetworkAudioSource : public AudioSource
{
private:
  NetworkChannelData *mChannelData;
  HTTPClient http;

public:
  NetworkAudioSource(NetworkChannelData *channelData);
  int getAudioSamples(uint8_t **buffer, size_t &bufferSize, int currentAudioSample);
};