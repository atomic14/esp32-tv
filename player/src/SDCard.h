#pragma once

#include <freertos/FreeRTOS.h>
#include <driver/sdmmc_types.h>
#include <driver/sdmmc_host.h>
#include <driver/sdspi_host.h>
#include <vector>
#include <string>


class SDCard
{
private:
  sdmmc_card_t *m_card;
  #ifdef USE_SDIO
    sdmmc_host_t m_host = SDMMC_HOST_DEFAULT();
  #else
    sdmmc_host_t m_host = SDSPI_HOST_DEFAULT();
  #endif
  bool sd_card_init_success = false;
public:
  SDCard(gpio_num_t miso, gpio_num_t mosi, gpio_num_t clk, gpio_num_t cs);
  SDCard(gpio_num_t clk, gpio_num_t cmd, gpio_num_t d0, gpio_num_t d1, gpio_num_t d2, gpio_num_t d3);
  ~SDCard();
  bool isMounted();
  std::vector<std::string> listFiles(const char *folder, const char *extension=NULL);
};