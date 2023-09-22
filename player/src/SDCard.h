#pragma once

#include <Arduino.h>
#include "SPI.h"
#include <vector>
#include <string>


class SDCard
{
private:
  SPIClass spi = SPIClass(HSPI);
public:
  SDCard(int miso, int mosi, int clk, int cs);
  ~SDCard();
  bool isMounted();
  std::vector<std::string> listFiles(const char *folder, const char *extension=NULL);
};