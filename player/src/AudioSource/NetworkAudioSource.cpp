#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "NetworkAudioSource.h"

NetworkAudioSource::NetworkAudioSource(const char *audioURL): mAudioURL(audioURL)
{
}

int NetworkAudioSource::getAudioSamples(int8_t *buffer, size_t maxSamples, int channel, int currentAudioSample)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    char urlBuffer[100];
    sprintf(urlBuffer, "%s/%d/%d/%d", mAudioURL, channel, currentAudioSample, maxSamples);
    http.begin(urlBuffer);
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK)
    {
      // read the audio data into the buffer
      int audioLength = http.getSize();
      http.getStreamPtr()->readBytes((uint8_t *)buffer, audioLength);
      return audioLength;
    }
  }
  return 0;
}
