#include "sdl_wrapper.h"
#include "body.h"
#include "color.h"
#include "text.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

const char WINDOW_TITLE[] = "CS 3";
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 500;
const double MS_PER_S = 1e3;
int w, h;

typedef struct body {
  list_t *polygon;
  char *image_path;
  text_t *text;
  rgb_color_t color;
  vector_t centroid;
  vector_t velocity;
  vector_t impulse;
  vector_t acceleration;
  vector_t force;
  double mass;
  double rotation;
  void *info;
  free_func_t info_freer;
  bool remove;
  bool just_hit;
} body_t;
/**
 * The coordinate at the center of the screen.
 */
vector_t center;
/**
 * The coordinate difference from the center to the top right corner.
 */
vector_t max_diff;
/**
 * The SDL window where the scene is rendered.
 */
SDL_Window *window;
/**
 * The renderer used to draw the scene.
 */
SDL_Renderer *renderer;
/**
 * The keypress handler, or NULL if none has been configured.
 */
key_handler_t key_handler = NULL;
/**
 * SDL's timestamp when a key was last pressed or released.
 * Used to mesasure how long a key has been held.
 */
uint32_t key_start_timestamp;
/**
 * The value of clock() when time_since_last_tick() was last called.
 * Initially 0.
 */
clock_t last_clock = 0;

/** Computes the center of the window in pixel coordinates */
vector_t get_window_center(void) {
  int *width = malloc(sizeof(*width)), *height = malloc(sizeof(*height));
  assert(width != NULL);
  assert(height != NULL);
  SDL_GetWindowSize(window, width, height);
  vector_t dimensions = {.x = *width, .y = *height};
  free(width);
  free(height);
  return vec_multiply(0.5, dimensions);
}

/**
 * Computes the scaling factor between scene coordinates and pixel coordinates.
 * The scene is scaled by the same factor in the x and y dimensions,
 * chosen to maximize the size of the scene while keeping it in the window.
 */
double get_scene_scale(vector_t window_center) {
  // Scale scene so it fits entirely in the window
  double x_scale = window_center.x / max_diff.x,
         y_scale = window_center.y / max_diff.y;
  return x_scale < y_scale ? x_scale : y_scale;
}

/** Maps a scene coordinate to a window coordinate */
vector_t get_window_position(vector_t scene_pos, vector_t window_center) {
  // Scale scene coordinates by the scaling factor
  // and map the center of the scene to the center of the window
  vector_t scene_center_offset = vec_subtract(scene_pos, center);
  double scale = get_scene_scale(window_center);
  vector_t pixel_center_offset = vec_multiply(scale, scene_center_offset);
  vector_t pixel = {.x = round(window_center.x + pixel_center_offset.x),
                    // Flip y axis since positive y is down on the screen
                    .y = round(window_center.y - pixel_center_offset.y)};
  return pixel;
}

/**
 * Converts an SDL key code to a char.
 * 7-bit ASCII characters are just returned
 * and arrow keys are given special character codes.
 */
char get_keycode(SDL_Keycode key) {
  switch (key) {
  case SDLK_LEFT:
    return LEFT_ARROW;
  case SDLK_UP:
    return UP_ARROW;
  case SDLK_RIGHT:
    return RIGHT_ARROW;
  case SDLK_DOWN:
    return DOWN_ARROW;
  case SDLK_SPACE:
    return SPACE;
  default:
    // Only process 7-bit ASCII characters
    return key == (SDL_Keycode)(char)key ? key : '\0';
  }
}

void sdl_init(vector_t min, vector_t max) {
  // Check parameters
  assert(min.x < max.x);
  assert(min.y < max.y);

  center = vec_multiply(0.5, vec_add(min, max));
  max_diff = vec_subtract(max, center);
  SDL_Init(SDL_INIT_EVERYTHING);
  int flags = IMG_INIT_JPG | IMG_INIT_PNG;
  IMG_Init(flags);

  // IMG_Init(IMG_INIT_PNG);
  window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
                            SDL_WINDOW_RESIZABLE);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
  TTF_Init();
}

bool sdl_is_done(state_t *state) {
  SDL_Event *event = malloc(sizeof(*event));
  assert(event != NULL);
  while (SDL_PollEvent(event)) {
    switch (event->type) {
    case SDL_QUIT:
      free(event);
      return true;
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      // Skip the keypress if no handler is configured
      // or an unrecognized key was pressed
      if (key_handler == NULL)
        break;
      char key = get_keycode(event->key.keysym.sym);
      if (key == '\0')
        break;

      uint32_t timestamp = event->key.timestamp;
      if (!event->key.repeat) {
        key_start_timestamp = timestamp;
      }
      key_event_type_t type =
          event->type == SDL_KEYDOWN ? KEY_PRESSED : KEY_RELEASED;
      double held_time = (timestamp - key_start_timestamp) / MS_PER_S;
      key_handler(key, type, held_time, state);
      break;
    }
  }
  free(event);
  return false;
}

void sdl_clear(void) {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);
}

void sdl_draw_polygon(list_t *points, rgb_color_t color) {
  // Check parameters
  size_t n = list_size(points);
  assert(n >= 3);
  assert(0 <= color.r && color.r <= 1);
  assert(0 <= color.g && color.g <= 1);
  assert(0 <= color.b && color.b <= 1);

  vector_t window_center = get_window_center();

  // Convert each vertex to a point on screen
  int16_t *x_points = malloc(sizeof(*x_points) * n),
          *y_points = malloc(sizeof(*y_points) * n);
  assert(x_points != NULL);
  assert(y_points != NULL);
  for (size_t i = 0; i < n; i++) {
    vector_t *vertex = list_get(points, i);
    vector_t pixel = get_window_position(*vertex, window_center);
    x_points[i] = pixel.x;
    y_points[i] = pixel.y;
  }

  // Draw polygon with the given color
  filledPolygonRGBA(renderer, x_points, y_points, n, color.r * 255,
                    color.g * 255, color.b * 255, 255);

  free(x_points);
  free(y_points);
}

void sdl_show(void) {
  // Draw boundary lines
  vector_t window_center = get_window_center();
  vector_t max = vec_add(center, max_diff),
           min = vec_subtract(center, max_diff);
  vector_t max_pixel = get_window_position(max, window_center),
           min_pixel = get_window_position(min, window_center);
  SDL_Rect *boundary = malloc(sizeof(*boundary));
  boundary->x = min_pixel.x;
  boundary->y = max_pixel.y;
  boundary->w = max_pixel.x - min_pixel.x;
  boundary->h = min_pixel.y - max_pixel.y;
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderDrawRect(renderer, boundary);
  free(boundary);

  SDL_RenderPresent(renderer);
}

void sdl_render_scene(scene_t *scene) {
  sdl_clear();
  size_t body_count = scene_bodies(scene);
  for (size_t i = 0; i < body_count; i++) {
    body_t *body = scene_get_body(scene, i);
    list_t *shape = body_get_shape(body);

    if (body->text != NULL) {
      SDL_Surface *surface_message = TTF_RenderText_Solid(
          get_font(body->text), get_phrase(body->text), get_color(body->text));
      SDL_Texture *message =
          SDL_CreateTextureFromSurface(renderer, surface_message);

      vector_t window_center = get_window_center();
      vector_t coords = get_window_position(get_loc(body->text), window_center);

      SDL_Rect message_rect;
      message_rect.x = coords.x;
      message_rect.y = coords.y;
      message_rect.w = surface_message->w;
      message_rect.h = surface_message->h;

      SDL_RenderCopy(renderer, message, NULL, &message_rect);
      SDL_FreeSurface(surface_message);
    } else if (body->image_path == NULL) {
      sdl_draw_polygon(shape, body_get_color(body));
    } else if (strcmp(body->image_path, "images/doodle.png") == 0) {
      SDL_Texture *img = IMG_LoadTexture(renderer, body->image_path);
      SDL_QueryTexture(img, NULL, NULL, &w, &h);

      vector_t window_center = get_window_center();
      vector_t coord = body_get_centroid(body);
      vector_t pixel = get_window_position(coord, window_center);

      SDL_Rect texr;
      texr.x = pixel.x - 25;
      texr.y = pixel.y - 45;
      texr.w = 70;
      texr.h = 50;
      SDL_RenderCopy(renderer, img, NULL, &texr);
      SDL_DestroyTexture(img);

    } else if (strcmp(body->image_path, "images/reg_platform.png") == 0 ||
               strcmp(body->image_path, "images/trap_platform.png") == 0 ||
               strcmp(body->image_path, "images/moving_platform.png") == 0) {
      SDL_Texture *img1 = IMG_LoadTexture(renderer, body->image_path);
      SDL_QueryTexture(img1, NULL, NULL, &w, &h);

      vector_t window_center = get_window_center();
      vector_t coord = body_get_centroid(body);
      vector_t pixel = get_window_position(coord, window_center);

      SDL_Rect texr1;
      texr1.x = pixel.x - 30;
      texr1.y = pixel.y - 10;
      texr1.w = 60;
      texr1.h = 20;

      SDL_RenderCopy(renderer, img1, NULL, &texr1);
      SDL_DestroyTexture(img1);
    } else if (strcmp(body->image_path, "images/monster.png") == 0) {
      SDL_Texture *img2 = IMG_LoadTexture(renderer, body->image_path);
      SDL_QueryTexture(img2, NULL, NULL, &w, &h);

      vector_t window_center = get_window_center();
      vector_t coord = body_get_centroid(body);
      vector_t pixel = get_window_position(coord, window_center);

      SDL_Rect texr2;
      texr2.x = pixel.x - 40;
      texr2.y = pixel.y - 50;
      texr2.w = 80;
      texr2.h = 100;

      SDL_RenderCopy(renderer, img2, NULL, &texr2);
      SDL_DestroyTexture(img2);
    } else if (strcmp(body->image_path, "images/spring_platform.png") == 0) {
      SDL_Texture *img1 = IMG_LoadTexture(renderer, body->image_path);
      SDL_QueryTexture(img1, NULL, NULL, &w, &h);

      vector_t window_center = get_window_center();
      vector_t coord = body_get_centroid(body);
      vector_t pixel = get_window_position(coord, window_center);

      SDL_Rect texr1;
      texr1.x = pixel.x - 35;
      texr1.y = pixel.y - 15;
      texr1.w = 60;
      texr1.h = 30;

      sdl_draw_polygon(shape, body_get_color(body));
      SDL_RenderCopy(renderer, img1, NULL, &texr1);
      SDL_DestroyTexture(img1);
    }

    list_free(shape);
  }
  sdl_show();
}

void sdl_on_key(key_handler_t handler) { key_handler = handler; }

double time_since_last_tick(void) {
  clock_t now = clock();
  double difference = last_clock
                          ? (double)(now - last_clock) / CLOCKS_PER_SEC
                          : 0.0; // return 0 the first time this is called
  last_clock = now;
  return difference;
}
