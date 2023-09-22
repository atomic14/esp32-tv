#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include "SDCard.h"


SDCard::SDCard(int miso, int mosi, int clk, int cs)
{
  spi.setFrequency(80000000);
  spi.begin(clk, miso, mosi, cs);

  if (!SD.begin(cs, spi, 80000000))
  {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC)
  {
    Serial.println("MMC");
  }
  else if (cardType == CARD_SD)
  {
    Serial.println("SDSC");
  }
  else if (cardType == CARD_SDHC)
  {
    Serial.println("SDHC");
  }
  else
  {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  Serial.printf("SD Card Mounted On: %s", SD.cardType() == CARD_MMC ? "MMC" : "SD");
}

SDCard::~SDCard()
{
  SD.end();
}

bool SDCard::isMounted() {
  return SD.cardType() != CARD_NONE;
}

std::vector<std::string> SDCard::listFiles(const char *folder, const char *extension)
{
  std::vector<std::string> files;
  Serial.printf("Listing directory: %s\n", folder);
  File root = SD.open(folder);
  if (!root)
  {
    Serial.println("Failed to open directory");
    return files;
  }
  if (!root.isDirectory())
  {
    Serial.println("Not a directory");
    return files;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (!file.isDirectory()) {
      std::string filename = file.name();
      bool isFile = !file.isDirectory();
      bool isVisible = filename[0] != '.';
      bool isMatchingExtension = extension == NULL || filename.find(extension) == filename.length() - strlen(extension);
      if (isFile && isVisible && isMatchingExtension) {
        files.push_back("/sd/" + filename);
      }
    }
    file = root.openNextFile();
  }
  return files;
}