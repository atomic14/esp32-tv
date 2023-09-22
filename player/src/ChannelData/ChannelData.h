#pragma once

class ChannelData
{
protected:
  int mChannelNumber = 0;
public:
  virtual int getChannelCount() = 0;
  virtual bool fetchChannelData() = 0;
  virtual void setChannel(int channel) = 0;
  int getChannelNumber() { return mChannelNumber; }
};