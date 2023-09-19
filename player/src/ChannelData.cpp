#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "ChannelData.h"

ChannelData::ChannelData(const char *channelInfoURL): mChannelInfoURL(channelInfoURL) {

}

bool ChannelData::fetchChannelData() {
  // check to see if we are connected to Wifi
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }
  // make a HTTP request to get the channel data
  HTTPClient http;
  http.begin(mChannelInfoURL);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    // read the response into a buffer
    int responseLength = http.getSize();
    char *responseBuffer = (char *)malloc(responseLength);
    http.getStreamPtr()->readBytes((uint8_t *)responseBuffer, responseLength);
    // parse the response
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, responseBuffer);
    if (error) {
      Serial.println("Failed to parse channel data");
      return false;
    }
    // get the channel count
    mChannelCount = doc.size();
    Serial.printf("Channel count: %d\n", mChannelCount);
    if (mChannelCount > MAX_CHANNEL_COUNT) {
      Serial.printf("Channel count %d is greater than max channel count %d\n", mChannelCount, MAX_CHANNEL_COUNT);
      mChannelCount = MAX_CHANNEL_COUNT;
    }
    // get the channel lengths
    for (int i=0; i<mChannelCount; i++) {
      mChannelLengths[i] = doc[i];
    }
    return true;
  } else {
    Serial.printf("HTTP error: %d\n", httpCode);
    return false;
  }
}
  
int ChannelData::getChannelCount() {
  return mChannelCount;
}

int ChannelData::getChannelLength(int channelIndex)
{
  return mChannelLengths[channelIndex % mChannelCount];
}