#include "color.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

rgb_color_t rand_color() {
  double rand_r = (double)(rand() % 10) / 10.0;
  double rand_g = (double)(rand() % 10) / 10.0;
  double rand_b = (double)(rand() % 10) / 10.0;

  rgb_color_t color = {rand_r, rand_g, rand_b};
  return color;
}

double get_r_color(rgb_color_t color) { return color.r; }

double get_g_color(rgb_color_t color) { return color.g; }

double get_b_color(rgb_color_t color) { return color.b; }

bool equals(rgb_color_t color1, rgb_color_t color2) {
  return (color1.r == color2.r) && (color1.g == color2.g) &&
         (color1.b == color2.b);
}