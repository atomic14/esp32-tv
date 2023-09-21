#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "AudioSource.h"

class NetworkAudioSource : public AudioSource
{
private:
  const char *mAudioURL = NULL;
  HTTPClient http;

public:
  NetworkAudioSource(const char *audioURL);
  int getAudioSamples(int8_t *buffer, size_t maxSamples, int channel, int currentAudioSample);
};