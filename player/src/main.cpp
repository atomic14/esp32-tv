#include <Arduino.h>
#include <WiFi.h>
#include <TFT_eSPI.h>
#include "RemoteInput.h"
#include "VideoPlayer.h"
#include "I2SOutput.h"
#include "ChannelData.h"

const char *FRAME_URL = "http://192.168.1.229:8123/frame";
const char *AUDIO_URL = "http://192.168.1.229:8123/audio";
const char *CHANNEL_INFO_URL = "http://192.168.1.229:8123/channel_info";

RemoteInput *remoteInput = NULL;
VideoPlayer *videoPlayer = NULL;
I2SOutput *audioOutput = NULL;
ChannelData *channelData = NULL;
TFT_eSPI tft = TFT_eSPI();

void setup()
{
  Serial.begin(115200);
  // connect to Wifi
  WiFi.begin("CMGResearch", "02087552867");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  WiFi.setSleep(false);
  Serial.println("");
  // disable WiFi power saving for speed
  Serial.println("WiFi connected");

  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.initDMA();
  tft.fillScreen(TFT_BLACK);
  tft.setTextFont(2);
  tft.setTextSize(2);


  remoteInput = new RemoteInput(IR_RECV_PIN, IR_RECV_PWR, IR_RECV_GND, IR_RECV_IND);
  remoteInput->start();

  // i2s speaker pins
  i2s_pin_config_t i2s_speaker_pins = {
      .bck_io_num = I2S_SPEAKER_SERIAL_CLOCK,
      .ws_io_num = I2S_SPEAKER_LEFT_RIGHT_CLOCK,
      .data_out_num = I2S_SPEAKER_SERIAL_DATA,
      .data_in_num = I2S_PIN_NO_CHANGE};
  audioOutput = new I2SOutput(I2S_NUM_1, i2s_speaker_pins);
  audioOutput->start(16000);

  videoPlayer = new VideoPlayer(FRAME_URL, AUDIO_URL, tft, audioOutput);
  videoPlayer->start();

  channelData = new ChannelData(CHANNEL_INFO_URL);
}

int channel = 0;
void loop()
{
  RemoteCommands command = remoteInput->getLatestCommand();
  if (command != RemoteCommands::UNKNOWN)
  {
    switch (command)
    {
    case RemoteCommands::POWER:
      videoPlayer->stop();
      tft.setCursor(20, 20);
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.println("TUNING...");
      Serial.println("POWER");
      // get the channel info
      while(!channelData->fetchChannelData()) {
        Serial.println("Failed to fetch channel data");
        delay(1000);
      }
      videoPlayer->setChannel(0, channelData->getChannelLength(0));
      videoPlayer->play();
      break;
    case RemoteCommands::VOLUME_UP:
      audioOutput->volumeUp();
      break;
    case RemoteCommands::VOLUME_DOWN:
      audioOutput->volumeDown();
      Serial.println("VOLUME_DOWN");
      break;
    case RemoteCommands::CHANNEL_UP:
      videoPlayer->playStatic();
      delay(500);
      channel = (channel + 1) % channelData->getChannelCount();
      videoPlayer->setChannel(channel, channelData->getChannelLength(channel));
      videoPlayer->play();
      Serial.printf("CHANNEL_UP %d\n", channel);
      break;
    case RemoteCommands::CHANNEL_DOWN:
      videoPlayer->playStatic();
      delay(500);
      channel--;
      if (channel < 0)
      {
        channel = channelData->getChannelCount() - 1;
      }
      videoPlayer->setChannel(channel, channelData->getChannelLength(channel));
      videoPlayer->play();
      Serial.printf("CHANNEL_DOWN %d\n", channel);
      break;
    }
    delay(100);
    remoteInput->getLatestCommand();
  }
  delay(100);
}
