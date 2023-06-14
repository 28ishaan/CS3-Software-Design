#include "forces.h"
#include "collision.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const double MIN_DIST = 5.0;
rgb_color_t const col_array[11] = {
    {1.0, 0.0, 0.0}, {0.9, 0.4, 0.0}, {1.0, 1.0, 0.0}, {0.5, 0.9, 0.5},
    {0.0, 1.0, 0.0}, {0.5, 1.0, 1.0}, {0.0, 0.0, 1.0}, {0.6, 0.1, 1.0},
    {1.0, 0.2, 0.6}, {1.0, 0.8, 0.9}, {0.8, 0.8, 0.8}};

typedef struct force {
  force_creator_t forcer;
  void *aux;
  free_func_t freer;
  list_t *bodies;
} force_t;

typedef struct aux {
  body_t *body1;
  body_t *body2;
  double constant;
} aux_t;

typedef struct aux_1 {
  body_t *body1;
  double constant;
} aux_1_t;

typedef struct collision_aux {
  body_t *body1;
  body_t *body2;
  void *aux;
  collision_handler_t handler;
  bool collided;
} collision_aux_t;

void force_free(force_t *force) {
  free(force->aux);
  list_free(force->bodies); // BE CAREFUL
  free(force);
}

list_t *force_get_bodies(force_t *force) { return force->bodies; }

force_creator_t get_forcer(force_t *force) { return force->forcer; }

void *get_aux(force_t *force) { return force->aux; }

free_func_t get_freer(force_t *force) { return force->freer; }

force_t *force_init(force_creator_t forcer, void *aux, free_func_t freer,
                    list_t *bodies) {
  force_t *new_force = malloc(sizeof(force_t));
  new_force->forcer = forcer;
  new_force->aux = aux;
  new_force->freer = freer;
  new_force->bodies = bodies;
  return new_force;
}

aux_t *aux_init(body_t *body1, body_t *body2, double constant) {
  aux_t *aux = malloc(sizeof(aux_t));
  aux->body1 = body1;
  aux->body2 = body2;
  aux->constant = constant;
  return aux;
}

aux_1_t *aux_1_init(body_t *body1, double constant) {
  aux_1_t *my_aux_1 = malloc(sizeof(aux_1_t));
  my_aux_1->body1 = body1;
  my_aux_1->constant = constant;
  return my_aux_1;
}

collision_aux_t *collision_aux_init(body_t *body1, body_t *body2, void *aux,
                                    collision_handler_t handler) {
  collision_aux_t *collision_aux = malloc(sizeof(collision_aux_t));
  collision_aux->body1 = body1;
  collision_aux->body2 = body2;
  collision_aux->aux = aux;
  collision_aux->handler = handler;
  collision_aux->collided = false;
  return collision_aux;
}

void gravity_creator(void *aux) {
  aux_t *aux1 = (aux_t *)aux;
  body_t *body1 = aux1->body1;
  body_t *body2 = aux1->body2;

  vector_t dist_vect =
      vec_subtract(body_get_centroid(body1), body_get_centroid(body2));
  double distance = sqrt(pow(dist_vect.x, 2.0) + pow(dist_vect.y, 2.0));
  if (distance <= MIN_DIST) {
    distance = MIN_DIST;
  }

  double force = aux1->constant * body_get_mass(body1) * body_get_mass(body2) /
                 pow(distance, 2.0);
  vector_t dir = vec_multiply(1 / distance, dist_vect);

  body_add_force(body1, vec_multiply(-force, dir));
  body_add_force(body2, vec_multiply(force, dir));
}

void spring_creator(void *aux) {
  aux_t *aux1 = (aux_t *)aux;
  body_t *body1 = aux1->body1;
  body_t *body2 = aux1->body2;

  vector_t dist_vect =
      vec_subtract(body_get_centroid(body1), body_get_centroid(body2));
  double distance = sqrt(pow(dist_vect.x, 2.0) + pow(dist_vect.y, 2.0));

  double force = aux1->constant * distance;
  vector_t dir = vec_multiply(1 / distance, dist_vect);

  body_add_force(body1, vec_multiply(-force, dir));
  body_add_force(body2, vec_multiply(force, dir));
}

void drag_creator(void *aux) {
  aux_1_t *aux1 = (aux_1_t *)aux;
  body_t *body = aux1->body1;
  vector_t velocity = body_get_velocity(body);
  body_add_force(body, vec_multiply(-1 * (aux1->constant), velocity));
}

void collision_creator(void *aux) {
  collision_aux_t *collision_aux = (collision_aux_t *)aux;
  body_t *body1 = collision_aux->body1;
  body_t *body2 = collision_aux->body2;

  list_t *shape1 = body_get_shape(body1);
  list_t *shape2 = body_get_shape(body2);

  collision_handler_t handler = collision_aux->handler;
  void *constant = collision_aux->aux;

  collision_info_t collision = find_collision(shape1, shape2);
  if (collision.collided && !collision_aux->collided) {
    collision_aux->collided = true;
    handler(body1, body2, collision.axis, constant);
  } else if (!collision.collided) {
    collision_aux->collided = false;
  }
  list_free(shape1);
  list_free(shape2);
}

void destructive_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                                   void *constant) {
  body_remove(body1);
  body_remove(body2);
}

void elastic_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                               void *constant) {
  double *elasticity = (double *)constant;
  double m1 = body_get_mass(body1);
  double m2 = body_get_mass(body2);
  double reduced_mass = (m1 * m2) / (m1 + m2);

  if (m1 == INFINITY) {
    reduced_mass = m2;
  } else if (m2 == INFINITY) {
    reduced_mass = m1;
  }

  double u_b = vec_dot(body_get_velocity(body2), axis);
  double u_a = vec_dot(body_get_velocity(body1), axis);

  double impulse1 = reduced_mass * (1.0 + *elasticity) * (u_b - u_a);
  double impulse2 = -impulse1;
  if (!body_get_just_hit(body1)) {
    body_add_impulse(body1, (vector_t)vec_multiply(impulse1, axis));
    body_add_impulse(body2, (vector_t)vec_multiply(impulse2, axis));
    body_set_just_hit(body1, true);
  }
}

void color_handler(body_t *body1, body_t *body2, vector_t axis, void *aux) {
  rgb_color_t *color = malloc(sizeof(rgb_color_t));
  *color = body_get_color(body1);

  for (size_t i = 0; i < 11; i++) {
    rgb_color_t curr = col_array[i];
    if (curr.r == color->r && curr.g == color->g && curr.b == color->b &&
        i == 10) {
      body_remove(body1);
    } else {
      body_set_color(body1, col_array[10]);
    }
  }
}

void create_newtonian_gravity(scene_t *scene, double G, body_t *body1,
                              body_t *body2) {
  aux_t *aux = aux_init(body1, body2, G);
  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  scene_add_bodies_force_creator(scene, gravity_creator, aux, bodies, NULL);
}

void create_spring(scene_t *scene, double k, body_t *body1, body_t *body2) {
  aux_t *aux = aux_init(body1, body2, k);
  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  scene_add_bodies_force_creator(scene, spring_creator, aux, bodies, NULL);
}

void create_drag(scene_t *scene, double gamma, body_t *body) {
  aux_1_t *my_aux_1 = aux_1_init(body, gamma);
  list_t *bodies = list_init(1, NULL);
  list_add(bodies, body);
  scene_add_bodies_force_creator(scene, drag_creator, my_aux_1, bodies, NULL);
}

void create_collision(scene_t *scene, body_t *body1, body_t *body2,
                      collision_handler_t handler, void *aux,
                      free_func_t freer) {
  collision_aux_t *info = collision_aux_init(body1, body2, aux, handler);
  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  scene_add_bodies_force_creator(scene, collision_creator, info, bodies, freer);
}

void create_destructive_collision(scene_t *scene, body_t *body1,
                                  body_t *body2) {
  create_collision(scene, body1, body2,
                   (collision_handler_t)destructive_collision_handler, NULL,
                   NULL);
}

void create_physics_collision(scene_t *scene, double elasticity, body_t *body1,
                              body_t *body2) {
  double *e = malloc(sizeof(double));
  *e = elasticity;
  create_collision(scene, body1, body2,
                   (collision_handler_t)elastic_collision_handler, (void *)e,
                   free);
}

void create_color_change(scene_t *scene, body_t *body1, body_t *body2) {
  create_collision(scene, body1, body2, (collision_handler_t)color_handler,
                   NULL, NULL);
}
