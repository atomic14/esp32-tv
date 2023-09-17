#pragma once

#define MAX_CHANNEL_COUNT 100

class ChannelData
{
private:
  const char *mChannelInfoURL = NULL;

  int mChannelCount = 0;
  int mChannelLengths[MAX_CHANNEL_COUNT] = {0};

public:
  ChannelData(const char *channelInfoURL);
  bool fetchChannelData();
  int getChannelCount();
  int getChannelLength(int channelIndex);
};