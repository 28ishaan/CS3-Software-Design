#ifndef __STAR_H__
#define __STAR_H__

#include "color.h"
#include "polygon.h"
#include "state.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct star star_t;

star_t *star_init(double side_length, vector_t *start_pos, size_t num_points,
                  rgb_color_t color, double x_vel, double y_vel);

list_t *make_star(double side_length, vector_t *start_pos, size_t num_points);

list_t *get_star_coords(star_t *star);

vector_t get_star_velocity(star_t *star);

size_t get_star_points(star_t *star);

bool get_star_bounced(star_t *star);

void set_star_bounced(star_t *star, bool cond);

void set_star_velocity(star_t *star, vector_t vec);

rgb_color_t get_star_color(star_t *star);

void free_star(star_t *star);

#endif // #ifndef __STAR_H__