#ifndef LED_MATRIX
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "TFT.h"

TFT::TFT(): tft(new TFT_eSPI()) {
  // power on the tft
  #ifdef TFT_POWER
  if (TFT_POWER != GPIO_NUM_NC) {
    Serial.println("Powering on TFT");
    pinMode(TFT_POWER, OUTPUT);
    digitalWrite(TFT_POWER, TFT_POWER_ON);
  }
  #endif

  tft->init();
  #ifdef M5CORE2
  tft->setRotation(6);
  #else
  tft->setRotation(1);
  #endif
  tft->fillScreen(TFT_BLACK);
  #ifdef USE_DMA
  tft->initDMA();
  #endif
  tft->fillScreen(TFT_BLACK);
  tft->setTextFont(2);
  tft->setTextSize(2);
  tft->setTextColor(TFT_GREEN, TFT_BLACK);
}

void TFT::drawPixels(int x, int y, int width, int height, uint16_t *pixels) {
  int numPixels = width * height;
  if (dmaBuffer[dmaBufferIndex] == NULL)
  {
    dmaBuffer[dmaBufferIndex] = (uint16_t *)malloc(numPixels * 2);
  }
  memcpy(dmaBuffer[dmaBufferIndex], pixels, numPixels * 2);
  #ifdef USE_DMA
  tft->dmaWait();
  #endif
  tft->setAddrWindow(x, y, width, height);
  #ifdef USE_DMA
  tft->pushPixelsDMA(dmaBuffer[dmaBufferIndex], numPixels);
  #else
  tft->pushPixels(dmaBuffer[dmaBufferIndex], numPixels);
  #endif
  dmaBufferIndex = (dmaBufferIndex + 1) % 2;
}

void TFT::startWrite() {
  tft->startWrite();
}

void TFT::endWrite() {
  tft->endWrite();
}

int TFT::width() {
  return tft->width();
}

int TFT::height() {
  return tft->height();
}

void TFT::fillScreen(uint16_t color) {
  tft->fillScreen(color);
}

void TFT::drawChannel(int channelIndex) {
  tft->setCursor(20, 20);
  tft->setTextColor(TFT_GREEN, TFT_BLACK);
  tft->printf("%d", channelIndex);
}

void TFT::drawTuningText() {
  tft->setCursor(20, 20);
  tft->setTextColor(TFT_GREEN, TFT_BLACK);
  tft->println("TUNING...");
}

void TFT::drawSDCardFailed() {
  tft->fillScreen(TFT_RED);
  tft->setCursor(0, 20);
  tft->setTextColor(TFT_WHITE);
  tft->setTextSize(2);
  tft->println("Failed to mount SD Card");
}

void TFT::drawFPS(int fps) {
    // show the frame rate in the top right
    tft->setCursor(width() - 50, 20);
    tft->setTextColor(TFT_GREEN, TFT_BLACK);
    tft->printf("%d", fps);
}
#endif