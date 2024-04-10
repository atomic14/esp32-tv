#ifdef LED_MATRIX

#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "Matrix.h"

Matrix::Matrix() {
  HUB75_I2S_CFG mxconfig(
    64,   // module width
    64,   // module height
    1
  );
  mxconfig.gpio.e = 18;
  mxconfig.clkphase = false;
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
}

void Matrix::drawPixels(int x, int y, int width, int height, uint16_t *pixels) {
  dma_display->drawRGBBitmap(x, y, pixels, width, height);
}

void Matrix::startWrite() {
  dma_display->startWrite();
}

void Matrix::endWrite() {
  dma_display->endWrite();
}

int Matrix::width() {
  return dma_display->width();
}

int Matrix::height() {
  return dma_display->height();
}

void Matrix::fillScreen(uint16_t color) {
  dma_display->fillScreen(color);
}

void Matrix::drawChannel(int channelIndex) {
  dma_display->setCursor(20, 20);
  dma_display->setTextColor(0xffff, 0x0000);
  dma_display->printf("%d", channelIndex);
}

void Matrix::drawTuningText() {
  dma_display->setCursor(20, 20);
  dma_display->setTextColor(0xffff, 0x0000);
  dma_display->println("TUNING...");
}

void Matrix::drawSDCardFailed() {
  // fill the screen with red
  dma_display->fillScreen(0xf800);
  dma_display->setCursor(20, 20);
  dma_display->setTextColor(0xffff, 0x0000);
  dma_display->println("SD Card Failed");
}

void Matrix::drawFPS(int fps) {
  // show the frame rate in the top right
  dma_display->setCursor(width() - 50, 20);
  dma_display->setTextColor(0xffff, 0x0000);
  dma_display->printf("%d", fps);
}

#endif