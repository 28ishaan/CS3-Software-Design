#ifndef __COLOR_H__
#define __COLOR_H__

#include <stdbool.h>
#include <stddef.h>

/**
 * A color to display on the screen.
 * The color is represented by its red, green, and blue components.
 * Each component must be between 0 (black) and 1 (white).
 */
typedef struct rgb_color {
  double r;
  double g;
  double b;
} rgb_color_t;

rgb_color_t rand_color();

double get_r_color(rgb_color_t color);

double get_g_color(rgb_color_t color);

double get_b_color(rgb_color_t color);

bool equals(rgb_color_t color1, rgb_color_t color2);

#endif // #ifndef __COLOR_H__
