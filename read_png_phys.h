#ifndef READ_PNG_PHYS_H_
#define READ_PNG_PHYS_H_

#include <stdio.h>
#include <stdint.h>

typedef struct PNGpHYs {
  uint32_t x_pixels_per_unit;
  uint32_t y_pixels_per_unit;
  uint8_t units; /* 0 - Unknown, 1 - meters */
} PNGpHYs;

typedef struct PNGpHYsResult {
  PNGpHYs value;
  const char *error;
} PNGpHYsResult;

PNGpHYsResult read_png_phys(FILE *file);
PNGpHYsResult read_png_phys_from_path(const char *file_path);

#endif /* READ_PNG_PHYS_H_ */