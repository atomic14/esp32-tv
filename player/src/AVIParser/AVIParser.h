#pragma once

enum class AVIChunkType
{
  VIDEO, AUDIO
};

class AVIParser
{
private:
  std::string mFileName;
  AVIChunkType mRequiredChunkType;
  FILE *mFile = NULL;
  long mMoviListPosition = 0;
  long mMoviListLength;

  bool isMoviListChunk(unsigned int chunkSize);

public:
  AVIParser(std::string fname, AVIChunkType requiredChunkType);
  ~AVIParser();
  bool open();
  size_t getNextChunk(uint8_t **buffer, size_t &bufferLength);
};