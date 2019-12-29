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

static TTF_Font *font = NULL;
typedef enum RenderMode {
  RENDER_MODE_NATIVE,
  RENDER_MODE_SCALED,
} RenderMode;
static RenderMode current_render_mode;

static int RenderText(const char *text, int x, int y) {
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
  y += RenderText("Current mode:", x, y);
  switch (current_render_mode) {
    case RENDER_MODE_SCALED:
      mode_text = "upscaled by the IPU from 320x240";
      break;
    case RENDER_MODE_NATIVE:
      mode_text = "native screen resolution 320x480";
      break;
  }

  RenderText(mode_text, x, y);
  y += screen->h / 3;

  y += 2 * RenderText("Controls:", x, y);
  y +=
      RenderText("  " CHANGE_MODE_KEY_LABEL " to change rendering mode.", x, y);
  RenderText("  " QUIT_KEY_LABEL " to quit. ", x, y);

  SDL_Flip(screen);
}

static void LoadFont(const char *font_path, unsigned int hdpi,
                     unsigned int vdpi) {
  if (font != NULL) {
    TTF_CloseFont(font);
    font = NULL;
  }
  font = TTF_OpenFontDPI(font_path, 12, hdpi, vdpi);
  if (font == NULL) {
    fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
    exit(1);
  }
}

static void SetRenderMode(RenderMode render_mode, const char *font_path) {
  current_render_mode = render_mode;
  switch (current_render_mode) {
    case RENDER_MODE_NATIVE:
      if (SDL_SetVideoMode(320, 480, 0, 0) == NULL) {
        fprintf(stderr, "SDL_SetVideoMode 320x480 error: %s\n", SDL_GetError());
        exit(1);
      }
      LoadFont(font_path, 72, 72 * 2);
      break;
    case RENDER_MODE_SCALED:
      if (SDL_SetVideoMode(320, 240, 0, 0) == NULL) {
        fprintf(stderr, "SDL_SetVideoMode 320x240 error: %s\n", SDL_GetError());
        exit(1);
      }
      LoadFont(font_path, 0, 0);
      break;
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
  SetRenderMode(RENDER_MODE_NATIVE, font_path);
  EventLoop(font_path);
  TTF_CloseFont(font);
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