#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <string>
#include "NetworkAudioSource.h"
#include "../ChannelData/NetworkChannelData.h"

#define SAMPLES_PER_CHUNK 16000

NetworkAudioSource::NetworkAudioSource(NetworkChannelData *channelData): mChannelData(channelData)
{
}

int NetworkAudioSource::getAudioSamples(int8_t **buffer, size_t &bufferSize, int currentAudioSample)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    // resize the buffer if needed
    if (bufferSize < SAMPLES_PER_CHUNK)
    {
      *buffer = (int8_t *)realloc(*buffer, SAMPLES_PER_CHUNK);
      bufferSize = SAMPLES_PER_CHUNK;
    }
    std::string url = mChannelData->getAudioURL() + "/" + std::to_string(currentAudioSample) + "/" + std::to_string(bufferSize);
    http.begin(url.c_str());
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK)
    {
      // read the audio data into the buffer
      int audioLength = http.getSize();
      if (audioLength > 0) {
        http.getStreamPtr()->readBytes((uint8_t *) *buffer, audioLength);
      }
      return audioLength;
    }
  }
  return 0;
}
