#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdlib.h>
#include <unistd.h>

#include "read_png_phys.h"

#ifndef QUIT_KEY_LABEL
#define QUIT_KEY_LABEL "ESC"
#endif

#ifndef CHANGE_MODE_KEY_LABEL
#define CHANGE_MODE_KEY_LABEL "RETURN"
#endif

static const SDL_Color kBlack = {0, 0, 0, 0};
static const SDL_Color kWhite = {255, 255, 255, 0};

static TTF_Font *font12 = NULL;
static TTF_Font *font10 = NULL;
static TTF_Font *font8 = NULL;
static TTF_Font **fonts[] = {&font12, &font10, &font8};
static const int font_sizes[] = {12, 10, 8};
static const int num_fonts = sizeof(fonts) / sizeof(fonts[0]);

static const char *kImagePaths128[] = {"lenna_128x128.png",
                                       "../lenna_128x128.png"};
static const char *kImagePaths256[] = {"lenna_128x256.png",
                                       "../lenna_128x256.png"};

typedef enum RenderMode {
  RENDER_MODE_NATIVE,
  RENDER_MODE_SCALED,
} RenderMode;
static RenderMode current_render_mode;

static SDL_Surface *image = NULL;

static void UnloadImage() {
  if (image == NULL) return;
  SDL_FreeSurface(image);
  image = NULL;
}

static void LoadImage(const char **image_paths) {
  const char *image_path = NULL;
  for (int i = 0; i < 2; ++i) {
    if (access(image_paths[i], F_OK) == 0) {
      image_path = image_paths[i];
      break;
    }
  }
  if (image_path == NULL) {
    fputs("image not found\n", stderr);
    return;
  }
  UnloadImage();
  image = IMG_Load(image_path);
  if (image == NULL) {
    fprintf(stderr, "IMG_Load error %s: %s\n", image_path, IMG_GetError());
    return;
  }
  PNGpHYsResult phys_result = read_png_phys_from_path(image_path);
  if (phys_result.error != NULL) {
    fprintf(stderr, "Error reading pHYs: %s\n", phys_result.error);
    return;
  }
  PNGpHYs phys = phys_result.value;
  fprintf(stderr, "pHYs: %s %u %u %u\n", image_path, phys.x_pixels_per_unit,
          phys.y_pixels_per_unit, phys.units);
}

static void UnloadFont(TTF_Font **font) {
  if (*font == NULL) return;
  TTF_CloseFont(*font);
  *font = NULL;
}

static void LoadFont(TTF_Font **font, const char *font_path, int size,
                     unsigned int hdpi, unsigned int vdpi) {
  *font = TTF_OpenFontDPI(font_path, size, hdpi, vdpi);
  if (*font == NULL) {
    fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
    exit(1);
  }
}

static void LoadFonts(const char *font_path, unsigned int hdpi,
                      unsigned int vdpi) {
  for (int i = 0; i < num_fonts; ++i) {
    LoadFont(fonts[i], font_path, font_sizes[i], hdpi, vdpi);
  }
}

static void UnloadFonts() {
  for (int i = 0; i < num_fonts; ++i) {
    UnloadFont(fonts[i]);
  }
}

static int RenderText(TTF_Font *font, const char *text, int x, int y) {
  SDL_Surface *text_surface = TTF_RenderText_Shaded(font, text, kBlack, kWhite);
  if (text_surface == NULL) {
    fprintf(stderr, "%s\n", TTF_GetError());
    exit(1);
  }
  SDL_Rect dest_rect = {x, y, 0, 0};
  SDL_BlitSurface(text_surface, NULL, SDL_GetVideoSurface(), &dest_rect);
  const int result = text_surface->h;
  SDL_FreeSurface(text_surface);
  return result;
}

static void Render() {
  const int vscale = current_render_mode == RENDER_MODE_NATIVE ? 2 : 1;

  SDL_Surface *screen = SDL_GetVideoSurface();
  SDL_FillRect(screen, NULL,
               SDL_MapRGB(screen->format, kWhite.r, kWhite.g, kWhite.b));

  const int width = screen->w;
  const int x = 2;
  int y = 2 * vscale;

  const char *mode_text;
  switch (current_render_mode) {
    case RENDER_MODE_SCALED:
      mode_text = "Current mode: 320x240 - upscaled by the IPU";
      break;
    case RENDER_MODE_NATIVE:
      mode_text = "Current mode: 320x480 - native resolution";
      break;
  }
  y += 2 * RenderText(font12, mode_text, x, y);
  y += RenderText(font12, "Controls:", x, y);
  y += RenderText(
      font12, "  " CHANGE_MODE_KEY_LABEL " to change rendering mode.", x, y);
  y += RenderText(font12, "  " QUIT_KEY_LABEL " to quit. ", x, y);
  y += screen->h / 24;

  const char demo_text_upper[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  const char demo_text_lower[] = "abcdefghijklmnopqrstuvwxyz";
  const char demo_text_numbers[] = "0123456789";

  for (int i = 0; i < num_fonts; ++i) {
    TTF_Font *font = *fonts[i];
    y += RenderText(font, demo_text_upper, x, y);
    y += RenderText(font, demo_text_lower, x, y);
    y += RenderText(font, demo_text_numbers, x, y);
    y += screen->h / 48;
  }

  y += RenderText(font10, "https://github.com/glebm/", x, y);
  RenderText(font10, "retrofw_scaling_demo", x, y);

  if (image != NULL) {
    SDL_Surface *tmp_image = SDL_DisplayFormat(image);
    if (tmp_image == NULL) {
      fprintf(stderr, "SDL_DisplayFormat error: %s\n", SDL_GetError());
      exit(1);
    }
    const int img_width = 128;
    const int img_height = 128 * vscale;
    SDL_Rect dest_rect = {screen->w - img_width - 2,
                          screen->h - img_height - 2 * vscale, 0, 0};
    SDL_BlitSurface(tmp_image, NULL, screen, &dest_rect);
    SDL_FreeSurface(tmp_image);
  }

  SDL_Flip(screen);
}

static void SetRenderMode(RenderMode render_mode, const char *font_path) {
  current_render_mode = render_mode;
  int hdpi, vdpi;
  const char **image_paths;
  switch (current_render_mode) {
    case RENDER_MODE_NATIVE:
      if (SDL_SetVideoMode(320, 480, 0, 0) == NULL) {
        fprintf(stderr, "SDL_SetVideoMode 320x480 error: %s\n", SDL_GetError());
        exit(1);
      }
      hdpi = 72;
      vdpi = 2 * hdpi;
      image_paths = kImagePaths256;
      break;
    case RENDER_MODE_SCALED:
      if (SDL_SetVideoMode(320, 240, 0, 0) == NULL) {
        fprintf(stderr, "SDL_SetVideoMode 320x240 error: %s\n", SDL_GetError());
        exit(1);
      }
      hdpi = 0;
      vdpi = 0;
      image_paths = kImagePaths128;
      break;
  }
  LoadFonts(font_path, hdpi, vdpi);
  LoadImage(image_paths);
  Render();
}

static void ChangeRenderMode(const char *font_path) {
  SetRenderMode((current_render_mode + 1) % 2, font_path);
}

static void EventLoop(const char *font_path) {
  SDL_Event event;
  do {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_KEYDOWN:
          switch (event.key.keysym.sym) {
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
              ChangeRenderMode(font_path);
              break;
            case SDLK_ESCAPE:
              return;
          }
          break;
        case SDL_QUIT:
          return;
        default:
          break;
      }
    }
    SDL_Delay(16);
  } while (1);
}

static void Run(const char *font_path) {
  if (SDL_Init(SDL_INIT_VIDEO) <= -1) {
    fprintf(stderr, "SDL_Init(SDL_INIT_VIDEO) error: %s\n", SDL_GetError());
    exit(1);
  }
  SDL_ShowCursor(0);
  if (TTF_Init() == -1) {
    fprintf(stderr, "TTF_Init error: %s\n", TTF_GetError());
    exit(1);
  }
  if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
    fprintf(stderr, "IMG_Init error: %s\n", IMG_GetError());
    exit(1);
  }

  SetRenderMode(RENDER_MODE_SCALED, font_path);
  EventLoop(font_path);

  UnloadImage();
  IMG_Quit();

  UnloadFonts();
  TTF_Quit();

  SDL_Quit();
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("usage: %s <font path>\n", argv[0]);
    return 64;
  }

  Run(argv[1]);
}