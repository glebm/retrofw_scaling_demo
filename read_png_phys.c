#include "read_png_phys.h"

#include <string.h>

typedef struct Uint32OrError {
  uint32_t value;
  const char *error;
} Uint32OrError;

typedef struct ChunkHeader {
  uint32_t len;
  char type[4];
} ChunkHeader;

typedef struct ChunkHeaderOrError {
  ChunkHeader value;
  const char *error;
} ChunkHeaderOrError;

static uint32_t buf_to_uint32(unsigned char data[4]) {
  return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
}

static Uint32OrError read_uint32(FILE *in) {
  Uint32OrError result;
  unsigned char data[4];
  if (fread(data, 1, 4, in) != 4) {
    result.error = "Premature end of file (expected a uint32_t)";
  } else {
    result.value = buf_to_uint32(data);
    result.error = NULL;
  }
  return result;
}

static ChunkHeaderOrError read_chunk_header(FILE *in) {
  ChunkHeaderOrError result;
  unsigned char data[8];
  if (fread(data, 1, 8, in) != 8) {
    result.error = "Premature end of file (expected a chunk header)";
  } else {
    result.value.len = buf_to_uint32(data);
    memcpy(result.value.type, data + 4, 4);
    result.error = NULL;
  }
  return result;
}

PNGpHYsResult read_png_phys(FILE *file) {
  PNGpHYsResult result;
  if (fseek(file, 8, SEEK_CUR) != 0) {
    result.error = "Premature end of file (expected PNG header)";
    return result;
  }
  while (1) {
    ChunkHeaderOrError header = read_chunk_header(file);
    if (header.error != NULL) {
      result.error = header.error;
      return result;
    }
    const char *t = header.value.type;
    /* pHYs header, if present, must come before IDAT */
    if (t[0] == 'I' && t[1] == 'D' && t[2] == 'A' && t[3] == 'D') break;
    if (t[0] == 'p' && t[1] == 'H' && t[2] == 'Y' && t[3] == 's') {
      Uint32OrError uint32_or_error = read_uint32(file);
      if (uint32_or_error.error) {
        result.error = uint32_or_error.error;
        return result;
      }
      result.value.x_pixels_per_unit = uint32_or_error.value;

      uint32_or_error = read_uint32(file);
      if (uint32_or_error.error) {
        result.error = uint32_or_error.error;
        return result;
      }
      result.value.y_pixels_per_unit = uint32_or_error.value;

      if (fread(&result.value.units, 1, 1, file) != 1) {
        result.error = "Premature end of file (expected the pHYs units byte)";
        return result;
      }

      result.error = NULL;
      return result;
    }
    if (fseek(file, header.value.len + 4 /* CRC */, SEEK_CUR) != 0) {
      result.error =
          "Premature end of file (expected complete chunk data and CRC)";
      return result;
    }
  }
  result.error = NULL;
  result.value.x_pixels_per_unit = 11811;
  result.value.y_pixels_per_unit = 11811;
  result.value.units = 1;
  result.error = NULL;
  return result;
}

PNGpHYsResult read_png_phys_from_path(const char *file_path) {
  PNGpHYsResult result;
  FILE *file = fopen(file_path, "rb");
  if (file == NULL) {
    result.error = "Error opening file";
    return result;
  }
  result = read_png_phys(file);
  fclose(file);
  return result;
}