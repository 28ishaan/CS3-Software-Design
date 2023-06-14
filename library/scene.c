#include "scene.h"
#include "forces.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

size_t const STARTING_BODIES = 20;
typedef struct scene {
  list_t *bodies;
  size_t size;
  size_t capacity;
  list_t *forces;
  list_t *free_bodies;
} scene_t;

scene_t *scene_init(void) {
  scene_t *scene = malloc(sizeof(scene_t));
  scene->bodies = list_init(STARTING_BODIES, body_free);
  scene->forces = list_init(1, force_free);
  scene->free_bodies = list_init(1, body_free);

  assert(scene != NULL);
  assert(scene->bodies != NULL);
  assert(scene->forces != NULL);

  scene->size = 0;
  scene->capacity = STARTING_BODIES;
  return scene;
}

void scene_free(scene_t *scene) {
  list_free(scene->forces);
  list_free(scene->bodies);
  list_free(scene->free_bodies);
  free(scene);
}

size_t scene_bodies(scene_t *scene) { return scene->size; }

body_t *scene_get_body(scene_t *scene, size_t index) {
  assert(index < scene->size);
  list_t *bodies = scene->bodies;
  body_t *body = list_get(bodies, index);
  return body;
}

void scene_add_body(scene_t *scene, body_t *body) {
  assert(body != NULL);
  if (scene->size >= scene->capacity) {
    scene->capacity = scene->capacity * 2;
    scene->bodies = realloc(scene->bodies, sizeof(body_t *) * scene->capacity);
  }
  list_add(scene->bodies, body);
  scene->size++;
}

void scene_remove_body(scene_t *scene, size_t index) {
  assert(index < scene->size);
  body_remove(scene_get_body(scene, index));
}

void scene_add_bodies_force_creator(scene_t *scene, force_creator_t forcer,
                                    void *aux, list_t *bodies,
                                    free_func_t freer) {
  force_t *force = force_init(forcer, aux, freer, bodies);
  list_add(scene->forces, force);
}

void scene_add_force_creator(scene_t *scene, force_creator_t forcer, void *aux,
                             free_func_t freer) {
  force_t *new_forcer = force_init(forcer, aux, freer, list_init(0, NULL));
  assert(new_forcer != NULL);
  list_add(scene->forces, new_forcer);
}

void scene_tick(scene_t *scene, double dt) {
  for (size_t i = 0; i < list_size(scene->forces); i++) {
    force_t *curr_force = (force_t *)list_get(scene->forces, i);
    force_creator_t my_force_creator = get_forcer(curr_force);
    my_force_creator(get_aux(curr_force));
  }

  for (size_t k = 0; k < list_size(scene->bodies); k++) {
    body_t *body = list_get(scene->bodies, k);
    if (body_is_removed(body)) {
      for (size_t i = 0; i < list_size(scene->forces); i++) {
        force_t *force = list_get(scene->forces, i);
        for (size_t j = 0; j < list_size(force_get_bodies(force)); j++) {
          if (body_is_removed(list_get(force_get_bodies(force), j))) {
            list_remove(scene->forces, i);
            force_free(force);
            i--;
            break;
          }
        }
      }
      list_remove(scene->bodies, k);
      list_add(scene->free_bodies, body);
      scene->size--;
      k--;
    } else {
      body_tick(body, dt);
    }
  }
}

void scene_shift_bodies(scene_t *scene, vector_t vec) {
  size_t num_bodies = scene_bodies(scene);
  assert(num_bodies > 1);
  for (size_t i = 1; i < num_bodies; i++) {
    body_t *curr_body = scene_get_body(scene, i);
    body_set_velocity(curr_body, (vector_t){0, -20});
  }
}

void set_all_bodies_to_zero(scene_t *scene) {
  size_t num_bodies = scene_bodies(scene);
  assert(num_bodies > 1);
  for (size_t i = 0; i < num_bodies; i++) {
    body_t *curr_body = scene_get_body(scene, i);
    body_set_velocity(curr_body, VEC_ZERO);
  }
}
