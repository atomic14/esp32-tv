#include <Arduino.h>
#include <WiFi.h>
#include <TFT_eSPI.h>
#include <driver/rtc_io.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_wifi.h>
#include "RemoteInput.h"
#include "VideoPlayer.h"
#include "audio_output/I2SOutput.h"
#include "audio_output/DACOutput.h"
#include "ChannelData/NetworkChannelData.h"
#include "ChannelData/SDCardChannelData.h"
#include "AudioSource/NetworkAudioSource.h"
#include "VideoSource/NetworkVideoSource.h"
#include "AudioSource/SDCardAudioSource.h"
#include "VideoSource/SDCardVideoSource.h"
#include "AVIParser/AVIParser.h"
#include "SDCard.h"

const char *WIFI_SSID = "CMGResearch";
const char *WIFI_PASSWORD = "02087552867";
const char *FRAME_URL = "http://192.168.1.229:8123/frame";
const char *AUDIO_URL = "http://192.168.1.229:8123/audio";
const char *CHANNEL_INFO_URL = "http://192.168.1.229:8123/channel_info";

#define ADC_EN 14
#define HW_EN   33
#define BUTTON_R 35
#define BUTTON_L 0

#ifdef HAS_IR_REMOTE
RemoteInput *remoteInput = NULL;
#else
#warning "No Remote Input - Will default to playing channel 0"
#endif

#ifndef USE_DMA
#warning "No DMA - Drawing may be slower"
#endif

VideoSource *videoSource = NULL;
AudioSource *audioSource = NULL;
VideoPlayer *videoPlayer = NULL;
AudioOutput *audioOutput = NULL;
ChannelData *channelData = NULL;
TFT_eSPI tft = TFT_eSPI();

void setup()
{
  Serial.begin(115200);
  pinMode(HW_EN, OUTPUT);
  digitalWrite(HW_EN, HIGH);  // step-up on
  pinMode(BUTTON_L, INPUT_PULLUP);
  pinMode(BUTTON_R, INPUT);
  delay(4000);
  #ifdef USE_SDCARD
  Serial.println("Using SD Card");
  SDCard *card = new SDCard(SD_CARD_MISO, SD_CARD_MOSI, SD_CARD_CLK, SD_CARD_CS);
  channelData = new SDCardChannelData(card, "/");
  audioSource = new SDCardAudioSource((SDCardChannelData *) channelData);
  videoSource = new SDCardVideoSource((SDCardChannelData *) channelData);
  #else
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  WiFi.setSleep(false);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  Serial.println("");
  // disable WiFi power saving for speed
  Serial.println("WiFi connected");
  channelData = new NetworkChannelData(CHANNEL_INFO_URL, FRAME_URL, AUDIO_URL);
  videoSource = new NetworkVideoSource((NetworkChannelData *) channelData);
  audioSource = new NetworkAudioSource((NetworkChannelData *) channelData);
  #endif

  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  #ifdef USE_DMA
  tft.initDMA();
  #endif
  tft.fillScreen(TFT_BLACK);
  tft.setTextFont(2);
  tft.setTextSize(2);

#ifdef HAS_IR_REMOTE
  remoteInput = new RemoteInput(IR_RECV_PIN, IR_RECV_PWR, IR_RECV_GND, IR_RECV_IND);
  remoteInput->start();
#endif

#ifdef USE_DAC_AUDIO
  audioOutput = new DACOutput(I2S_NUM_0);
  audioOutput->start(16000);
#else
  // i2s speaker pins
  i2s_pin_config_t i2s_speaker_pins = {
      .bck_io_num = I2S_SPEAKER_SERIAL_CLOCK,
      .ws_io_num = I2S_SPEAKER_LEFT_RIGHT_CLOCK,
      .data_out_num = I2S_SPEAKER_SERIAL_DATA,
      .data_in_num = I2S_PIN_NO_CHANGE};

  audioOutput = new I2SOutput(I2S_NUM_1, i2s_speaker_pins);
  audioOutput->start(16000);
#endif
  videoPlayer = new VideoPlayer(
    channelData,
    videoSource,
    audioSource,
    tft,
    audioOutput
  );
  videoPlayer->start();
#ifndef HAS_IR_REMOTE
  // no remote so we just play the first channel
  tft.setCursor(20, 20);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.println("TUNING...");
  // get the channel info
  while(!channelData->fetchChannelData()) {
    Serial.println("Failed to fetch channel data");
    delay(1000);
  }
  // default to first channel
  videoPlayer->setChannel(0);
  videoPlayer->play();
#endif
}

void powerDeepSeep() {
  digitalWrite(ADC_EN, LOW);
  delay(10);
  rtc_gpio_init(GPIO_NUM_14);
  rtc_gpio_set_direction(GPIO_NUM_14, RTC_GPIO_MODE_OUTPUT_ONLY);
  rtc_gpio_set_level(GPIO_NUM_14, 1);
  esp_bluedroid_disable();
  esp_bt_controller_disable();
  esp_wifi_stop();
  esp_deep_sleep_disable_rom_logging();
  // esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
  // esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
  // esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  delay(1000);
  esp_deep_sleep_start();
}

bool buttonToggle(){
  if (digitalRead(BUTTON_R) == 0) return true;
  else return false;
}

bool buttonPowerOff(){
  if (digitalRead(BUTTON_L) == 0) return true;
  else return false;
}

int channel = 0;
void loop()
{
#ifdef HAS_IR_REMOTE
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
      videoPlayer->setChannel(0);
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
      videoPlayer->setChannel(channel);
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
      videoPlayer->setChannel(channel);
      videoPlayer->play();
      Serial.printf("CHANNEL_DOWN %d\n", channel);
      break;
    }
    delay(100);
    remoteInput->getLatestCommand();
  }
#else
  if (buttonToggle()) {
    Serial.printf("CHANNEL_DOWN %d\n", channel);
    videoPlayer->playStatic();
    delay(500);
    channel--;
    if (channel < 0) {
      channel = channelData->getChannelCount() - 1;
    }
    videoPlayer->setChannel(channel);
    videoPlayer->play();
  }
  if (buttonPowerOff()){
    powerDeepSeep();
  }
#endif
  delay(100);
}
