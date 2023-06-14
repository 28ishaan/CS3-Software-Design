#include "star.h"
#include "list.h"
#include "polygon.h"
#include "state.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct star {
  list_t *star_coords;
  vector_t velocity;
  size_t num_points;
  bool bounced;
  rgb_color_t color;
} star_t;

/**
 * Makes the star
 *
 * Returns the list of coordinates
 */
list_t *make_star(double side_length, vector_t *start_pos, size_t num_points) {
  list_t *star_coords = list_init(num_points * 2, free);

  double const rotation_angle = 2 * M_PI / num_points;

  for (size_t i = 0; i < num_points + 1; i++) {
    vector_t *first_point = malloc(sizeof(vector_t));
    first_point->x = start_pos->x;
    first_point->y = start_pos->y + side_length;

    vector_t *second_point = malloc(sizeof(vector_t));
    double new_angle = 180 - (180 / num_points) - (90 / num_points);
    double val = side_length * sin(90 / num_points * (M_PI / 180)) /
                 sin((new_angle) * (M_PI / 180));
    double angle = (90 - 180 / num_points) * (M_PI / 180);
    second_point->x = start_pos->x - val * cos(angle);
    second_point->y = start_pos->y + val * sin(angle);

    list_add(star_coords, first_point);
    list_add(star_coords, second_point);

    polygon_rotate(star_coords, (-1.0) * rotation_angle, *start_pos);
  }

  return star_coords;
}

star_t *star_init(double side_length, vector_t *start_pos, size_t num_points,
                  rgb_color_t color, double x_vel, double y_vel) {
  star_t *star = malloc(sizeof(star_t));
  vector_t vel = {x_vel, y_vel};
  star->velocity = vel;
  star->num_points = num_points;
  star->star_coords = make_star(side_length, start_pos, num_points);
  star->bounced = false;
  star->color = color;
  return star;
}

list_t *get_star_coords(star_t *star) { return star->star_coords; }

vector_t get_star_velocity(star_t *star) { return star->velocity; }

size_t get_star_points(star_t *star) { return star->num_points; }

bool get_star_bounced(star_t *star) { return star->bounced; }

void set_star_bounced(star_t *star, bool cond) { star->bounced = cond; }

void set_star_velocity(star_t *star, vector_t vec) {
  star->velocity.x = vec.x;
  star->velocity.y = vec.y;
}

rgb_color_t get_star_color(star_t *star) { return star->color; }

void free_star(star_t *star) {
  list_free(star->star_coords);
  free(star);
}