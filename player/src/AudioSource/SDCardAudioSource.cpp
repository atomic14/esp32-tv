#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "SDCardAudioSource.h"
#include "../AVIParser/AVIParser.h"
#include "../ChannelData/SDCardChannelData.h"

SDCardAudioSource::SDCardAudioSource(SDCardChannelData *channelData): mChannelData(channelData)
{
}

int SDCardAudioSource::getAudioSamples(int8_t **buffer, size_t &bufferSize, int currentAudioSample)
{
  // read the audio data into the buffer
  AVIParser *parser = mChannelData->getAudioParser();
  if (parser) {
    int audioLength = parser->getNextChunk((uint8_t **) buffer, bufferSize);
    // conver the audio from unsigned to signed
    for (int i = 0; i < audioLength; i++) {
      (*buffer)[i] = ((uint8_t)(*buffer)[i]) - 128;
    }
    return audioLength;
  }
  return 0;
}
