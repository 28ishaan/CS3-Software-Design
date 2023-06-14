#include "body.h"
#include "color.h"
#include "polygon.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

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

body_t *body_init(list_t *shape, double mass, rgb_color_t color) {
  return body_init_with_info(shape, mass, color, NULL, NULL, NULL, NULL);
}

body_t *body_init_with_info(list_t *shape, double mass, rgb_color_t color,
                            char *image_path, text_t *text, void *info,
                            free_func_t info_freer) {
  body_t *body = malloc(sizeof(body_t));
  body->polygon = shape;
  body->image_path = image_path;
  body->text = text;
  body->color = color;
  body->mass = mass;
  body->velocity = (vector_t){0, 0};
  body->centroid = polygon_centroid(body->polygon);
  body->acceleration = (vector_t){0, 0};
  body->force = (vector_t){0, 0};
  body->impulse = (vector_t){0, 0};
  body->info = info;
  body->info_freer = info_freer;
  body->remove = false;
  body->just_hit = false;
  return body;
}

void body_free(body_t *body) {
  assert(body != NULL);
  list_free(body->polygon);
  free_func_t free_info = (free_func_t)body->info_freer;
  if (free_info != NULL) {
    free_info(body->info);
  }
  free(body);
}

list_t *body_get_shape(body_t *body) {
  assert(body->polygon != NULL);
  list_t *temp = list_init(list_size(body->polygon), free);
  for (size_t i = 0; i < list_size(body->polygon); i++) {
    vector_t *vec = malloc(sizeof(vector_t));
    *vec = *(vector_t *)list_get(body->polygon, i);
    list_add(temp, vec);
  }
  return temp;
}

vector_t body_get_centroid(body_t *body) { return body->centroid; }

vector_t body_get_velocity(body_t *body) { return body->velocity; }

rgb_color_t body_get_color(body_t *body) { return body->color; }

void *body_get_info(body_t *body) { return body->info; }

void body_set_color(body_t *body, rgb_color_t color) { body->color = color; }

void body_add_image(body_t *body, char *image_path) {
  body->image_path = image_path;
}

char *body_get_image(body_t *body) { return body->image_path; }

void body_add_text(body_t *body, text_t *text) { body->text = text; }

text_t *body_get_text(body_t *body) { return body->text; }

void body_set_centroid(body_t *body, vector_t x) {
  vector_t centroid = body->centroid;
  body->centroid = x;
  list_t *poly = body->polygon;
  polygon_translate(poly, vec_subtract(x, centroid));
}

void body_set_velocity(body_t *body, vector_t v) { body->velocity = v; }

void body_set_rotation(body_t *body, double angle) {
  polygon_rotate(body->polygon, -1 * body->rotation, body->centroid);
  polygon_rotate(body->polygon, angle, body->centroid);
  body->rotation = angle;
}

void body_add_force(body_t *body, vector_t force) {
  body->force = vec_add(body->force, force);
}

void body_add_impulse(body_t *body, vector_t impulse) {
  body->impulse = vec_add(body->impulse, impulse);
}

double body_get_mass(body_t *body) { return body->mass; }

void body_set_mass(body_t *body, double new_mass) { body->mass = new_mass; }

void body_tick(body_t *body, double dt) {

  assert(body->mass != 0.0);
  vector_t old_velocity = body->velocity;
  body->acceleration = vec_multiply(1.0 / body->mass, body->force);

  vector_t velocity_change = vec_multiply(1.0 / body->mass, body->impulse);
  vector_t new_velocity =
      vec_add(body->velocity, vec_multiply(dt, body->acceleration));
  body->velocity = vec_add(new_velocity, velocity_change);

  vector_t avg_velocity =
      vec_multiply(0.5, vec_add(old_velocity, body->velocity));
  // assert(avg_velocity.x != 0.0 && avg_velocity.y != 0.0);

  body_set_centroid(body,
                    vec_add(body->centroid, vec_multiply(dt, avg_velocity)));
  // polygon_rotate(body->polygon, body->rotation * dt, body->centroid);
  body->force = (vector_t){0, 0};
  body->impulse = (vector_t){0, 0};
  body_set_just_hit(body, false);
}

void body_remove(body_t *body) { body->remove = true; }

bool body_is_removed(body_t *body) { return body->remove; }

bool body_get_just_hit(body_t *body) { return body->just_hit; }

void body_set_just_hit(body_t *body, bool condition) {
  body->just_hit = condition;
}