#include <SDL.h>
#include <SDL_ttf.h>
#include <stdlib.h>

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

typedef enum RenderMode {
  RENDER_MODE_NATIVE,
  RENDER_MODE_SCALED,
} RenderMode;
static RenderMode current_render_mode;

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
  SDL_Surface *screen = SDL_GetVideoSurface();
  SDL_FillRect(screen, NULL,
               SDL_MapRGB(screen->format, kWhite.r, kWhite.g, kWhite.b));

  const int width = screen->w;
  const int x = screen->w / 20;
  int y = screen->h / 20;

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

  RenderText(font10, "https://github.com/glebm/retrofw_scaling_demo",
             x + screen->w / 10, y);

  SDL_Flip(screen);
}

static void LoadFont(TTF_Font **font, const char *font_path, int size,
                     unsigned int hdpi, unsigned int vdpi) {
  if (*font != NULL) {
    TTF_CloseFont(*font);
    *font = NULL;
  }
  *font = TTF_OpenFontDPI(font_path, size, hdpi, vdpi);
  if (*font == NULL) {
    fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
    exit(1);
  }
}

static void SetRenderMode(RenderMode render_mode, const char *font_path) {
  current_render_mode = render_mode;
  int hdpi, vdpi;
  switch (current_render_mode) {
    case RENDER_MODE_NATIVE:
      if (SDL_SetVideoMode(320, 480, 0, 0) == NULL) {
        fprintf(stderr, "SDL_SetVideoMode 320x480 error: %s\n", SDL_GetError());
        exit(1);
      }
      hdpi = 72;
      vdpi = 2 * hdpi;
      break;
    case RENDER_MODE_SCALED:
      if (SDL_SetVideoMode(320, 240, 0, 0) == NULL) {
        fprintf(stderr, "SDL_SetVideoMode 320x240 error: %s\n", SDL_GetError());
        exit(1);
      }
      hdpi = 0;
      vdpi = 0;
      break;
  }
  for (int i = 0; i < num_fonts; ++i) {
    LoadFont(fonts[i], font_path, font_sizes[i], hdpi, vdpi);
  }
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
  if (TTF_Init() == -1) {
    fprintf(stderr, "TTF_Init error: %s\n", TTF_GetError());
    exit(1);
  }
  SetRenderMode(RENDER_MODE_SCALED, font_path);
  EventLoop(font_path);
  TTF_CloseFont(font12);
  TTF_CloseFont(font10);
  TTF_CloseFont(font8);
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