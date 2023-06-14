#ifndef __TEXT_H__
#define __TEXT_H__

#include "list.h"
#include "vector.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct text {
  char *phrase;
  TTF_Font *font;
  SDL_Color color;
  vector_t loc;
  free_func_t freer;
} text_t;

text_t *text_init(char *phrase, TTF_Font *font, SDL_Color color, vector_t loc,
                  free_func_t freer);

TTF_Font *get_font(text_t *text);

char *get_phrase(text_t *text);

SDL_Color get_color(text_t *text);

vector_t get_loc(text_t *text);

#endif // #ifndef __TEXT_H__
