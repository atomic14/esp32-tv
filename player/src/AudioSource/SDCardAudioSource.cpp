#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "SDCardAudioSource.h"
#include "../AVIParser/AVIParser.h"
#include "../ChannelData/SDCardChannelData.h"

SDCardAudioSource::SDCardAudioSource(SDCardChannelData *channelData): mChannelData(channelData)
{
}

int SDCardAudioSource::getAudioSamples(uint8_t **buffer, size_t &bufferSize, int currentAudioSample)
{
  // read the audio data into the buffer
  AVIParser *parser = mChannelData->getAudioParser();
  if (parser) {
    int audioLength = parser->getNextChunk((uint8_t **) buffer, bufferSize);
    return audioLength;
  }
  return 0;
}
