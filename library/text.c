#include "text.h"
#include "vector.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

text_t *text_init(char *phrase, TTF_Font *font, SDL_Color color, vector_t loc,
                  free_func_t freer) {
  text_t *n_text = malloc(sizeof(text_t));
  n_text->phrase = phrase;
  n_text->font = font;
  n_text->color = color;
  n_text->loc = loc;
  n_text->freer = freer;
  return n_text;
}

TTF_Font *get_font(text_t *text) { return text->font; }

char *get_phrase(text_t *text) { return text->phrase; }

SDL_Color get_color(text_t *text) { return text->color; }

vector_t get_loc(text_t *text) { return text->loc; }