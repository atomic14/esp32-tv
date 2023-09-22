#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "NetworkChannelData.h"

NetworkChannelData::NetworkChannelData(const char *channelInfoURL, const char *frameURL, const char *audioURL) : mChannelInfoURL(channelInfoURL), mFrameURL(frameURL), mAudioURL(audioURL) {

}

bool NetworkChannelData::fetchChannelData() {
  // check to see if we are connected to Wifi
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }
  // make a HTTP request to get the channel data
  HTTPClient http;
  http.begin(mChannelInfoURL.c_str());
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
    // get the channel lengths
    for (int i=0; i<doc.size(); i++) {
      mChannelLengths.push_back(doc[i]);
    }
    return true;
  } else {
    Serial.printf("HTTP error: %d\n", httpCode);
    return false;
  }
}

std::string NetworkChannelData::getFrameURL() {
  return mFrameURL + "/" + std::to_string(mChannelNumber);
}

std::string NetworkChannelData::getAudioURL() {
  return mAudioURL + "/" + std::to_string(mChannelNumber);
}

void NetworkChannelData::setChannel(int channel) {
  mChannelNumber = channel;
}