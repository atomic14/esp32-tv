#pragma once

#include "ChannelData.h"
#include <vector>
#include <string>

class SDCard;
class AVIParser;

class SDCardChannelData : public ChannelData
{
private:
  const char *mChannelInfoURL = NULL;
  std::vector<std::string> mAviFiles;
  AVIParser *mCurrentChannelAudioParser = NULL;
  AVIParser *mCurrentChannelVideoParser = NULL;
  SDCard *mSDCard;
  const char *mAviPath;
public:
  SDCardChannelData(SDCard *sdCard, const char *aviPath);
  bool fetchChannelData();
  int getChannelCount() {
    return mAviFiles.size();
  };
  int getChannelLength(int channelIndex) {
    // we don't know the length of the AVI file
    return -1;
  };
  AVIParser *getAudioParser() {
    return mCurrentChannelAudioParser;
  };
  AVIParser *getVideoParser() {
    return mCurrentChannelVideoParser;
  };
  void setChannel(int channel);
};