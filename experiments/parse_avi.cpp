#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char chunkId[4];
    unsigned int chunkSize;
} ChunkHeader;

void readChunk(FILE *file, ChunkHeader *header) {
    fread(&header->chunkId, 4, 1, file);
    fread(&header->chunkSize, 4, 1, file);
    printf("ChunkId %c%c%c%c, size %u\n",
           header->chunkId[0], header->chunkId[1],
           header->chunkId[2], header->chunkId[3],
           header->chunkSize);
}

void processMovieList(FILE *fp, unsigned int chunkSize) {
    ChunkHeader header;
    while (chunkSize > 0) {
        readChunk(fp, &header);
        if (strncmp(header.chunkId, "00dc", 4) == 0) {
            printf("Found video frame.\n");
            // skip the frame data bytes
            fseek(fp, header.chunkSize, SEEK_CUR);
        } else if (strncmp(header.chunkId, "01wb", 4) == 0) {
            printf("Found audio data.\n");
            // skip the audio data bytes
            fseek(fp, header.chunkSize, SEEK_CUR);
        } else {
            // skip the chunk data bytes
            fseek(fp, header.chunkSize, SEEK_CUR);
        }
        chunkSize -= 8 + header.chunkSize;
        // handle any padding bytes
        if (header.chunkSize % 2 != 0) {
            fseek(fp, 1, SEEK_CUR);
            chunkSize--;
        }
    }
}

void processListChunk(FILE *fp, unsigned int chunkSize) {
    char listType[4];
    fread(&listType, 4, 1, fp);
    chunkSize -= 4;
    printf("LIST type %c%c%c%c\n",
           listType[0], listType[1],
           listType[2], listType[3]);
    // check for the movi list - contains the video frames and audio data
    if (strncmp(listType, "movi", 4) == 0) {
        printf("Found movi list.\n");
        processMovieList(fp, chunkSize);
    } else {
        // skip the rest of the bytes
        fseek(fp, chunkSize, SEEK_CUR);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <avi_file>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        printf("Failed to open file.\n");
        return 1;
    }

    ChunkHeader header;

    // Read RIFF header
    readChunk(file, &header);
    if (strncmp(header.chunkId, "RIFF", 4) != 0) {
        printf("Not a valid AVI file.\n");
        return 1;
    } else {
        printf("RIFF header found.\n");
    }


    // next four bytes are the RIFF type which should be 'AVI '
    char riffType[4];
    fread(&riffType, 4, 1, file);
    if (strncmp(riffType, "AVI ", 4) != 0) {
        printf("Not a valid AVI file.\n");
        return 1;
    } else {
        printf("RIFF Type is AVI.\n");
    }

    // now read each chunk until the end of the file

    while (!feof(file) && !ferror(file)) {
        readChunk(file, &header);
        if (feof(file) || ferror(file)) {
            break;
        }
        // is it a LIST chunk?
        if (strncmp(header.chunkId, "LIST", 4) == 0) {
            processListChunk(file, header.chunkSize);
        } else {
            // skip the chunk data bytes
            fseek(file, header.chunkSize, SEEK_CUR);
        }
    }
    fclose(file);
    return 0;
}
