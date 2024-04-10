#pragma once

#include "Display.h"

class MatrixPanel_I2S_DMA;

class Matrix: public Display {
private:
  MatrixPanel_I2S_DMA *dma_display = nullptr;
public:
  Matrix();
  void drawPixels(int x, int y, int width, int height, uint16_t *pixels);
  void startWrite();
  void endWrite();
  int width();
  int height();
  void fillScreen(uint16_t color);
  void drawChannel(int channelIndex);
  void drawTuningText();
  void drawFPS(int fps);
  void drawSDCardFailed();
};