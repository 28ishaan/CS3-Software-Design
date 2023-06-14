#include "body.h"
#include "collision.h"
#include "forces.h"
#include "list.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "state.h"
#include "vec_list.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

size_t const RECT_SIDES = 4;
size_t const BALL_POINTS = 1000;
double const BALL_RADIUS = 10.0;
double const RECT_WIDTH = 47.25;
double const RECT_HEIGHT = 10;
double const PLAYER_WIDTH = 75;
double const PLAYER_HEIGHT = 10;
double const XMIN = 0.0;
double const XMAX = 1000.0;
double const YMIN = 0.0;
double const YMAX = 500.0;
size_t const ROWS = 3;
size_t const COLS = 10;
double const BLOCK_MASS = INFINITY;
double const BALL_MASS = 1.0;
size_t const DOWN_SHIFT = 5;
size_t const RIGHT_SHIFT = 5;
rgb_color_t const PLAYER_COLOR = {1.0, 0.2, 0.6};
vector_t const INIT_BALL_VEL = {123.0, 550.0};
size_t const COLOR_SIZE = 11;
const vector_t RVELO = {600, 0};
const vector_t LVELO = {-600, 0};
const size_t SMALL_SHIFT = 5;
double const ELASTICITY = 1.0;
const size_t BALL_COLOR = 5;

typedef struct state {
  scene_t *scene;
  size_t num_blocks;
  double total_time;
  vector_t start;
} state_t;

rgb_color_t const color_array[COLOR_SIZE] = {
    {1.0, 0.0, 0.0}, {0.9, 0.4, 0.0}, {1.0, 1.0, 0.0}, {0.5, 0.9, 0.5},
    {0.0, 1.0, 0.0}, {0.5, 1.0, 1.0}, {0.0, 0.0, 1.0}, {0.6, 0.1, 1.0},
    {1.0, 0.2, 0.6}, {1.0, 0.8, 0.9}, {0.8, 0.8, 0.8}};

body_t *make_rect(double width, double height) {
  list_t *rect = list_init(RECT_SIDES, free);
  vector_t *vec = malloc(sizeof(vector_t));
  *vec = (vector_t){width, height};
  list_add(rect, vec);
  vec = malloc(sizeof(vector_t));
  *vec = (vector_t){-width, height};
  list_add(rect, vec);
  vec = malloc(sizeof(vector_t));
  *vec = (vector_t){-width, -height};
  list_add(rect, vec);
  vec = malloc(sizeof(vector_t));
  *vec = (vector_t){width, -height};
  list_add(rect, vec);

  body_t *block = body_init(rect, BLOCK_MASS, color_array[0]);
  return block;
}

void make_all_rects(state_t *state) {
  double x_pos = XMIN + RECT_WIDTH + RIGHT_SHIFT;
  double y_pos = YMAX - RECT_HEIGHT;
  double shift = 2 * RECT_HEIGHT + DOWN_SHIFT;
  size_t color_index = 0;
  for (size_t i = 0; i < ROWS; i++) {
    for (size_t j = 0; j < COLS; j++) {
      body_t *block = make_rect(RECT_WIDTH, RECT_HEIGHT);
      body_set_color(block, color_array[color_index % (COLOR_SIZE - 1)]);
      color_index++;
      body_set_centroid(block, (vector_t){x_pos, y_pos});
      scene_add_body(state->scene, block);
      state->num_blocks++;
      x_pos += RECT_WIDTH * 2 + RIGHT_SHIFT;
    }
    y_pos -= shift;
    x_pos = XMIN + RECT_WIDTH + RIGHT_SHIFT;
  }
}

void make_player(state_t *state) {
  body_t *player = make_rect(PLAYER_WIDTH, PLAYER_HEIGHT);
  body_set_color(player, PLAYER_COLOR);
  body_set_centroid(player, (vector_t){XMAX / 2.0, YMAX / 10.0});
  scene_add_body(state->scene, player);
}

void make_ball(state_t *state) {
  list_t *ball_coords = list_init(BALL_POINTS, free);
  vector_t start = {XMAX / 2.0, YMAX / 10.0 + BALL_RADIUS * 2 + SMALL_SHIFT};

  double const rotation_angle = 2 * M_PI / BALL_POINTS;

  for (size_t i = 0; i < BALL_POINTS; i++) {
    double angle = i * rotation_angle;

    vector_t *vec = malloc(sizeof(vector_t));
    vec->x = BALL_RADIUS * cos(angle);
    vec->y = BALL_RADIUS * sin(angle);
    *vec = vec_add(*vec, start);

    list_add(ball_coords, vec);
  }

  body_t *ball = body_init(ball_coords, BALL_MASS, color_array[BALL_COLOR]);
  body_set_velocity(ball, (vector_t){INIT_BALL_VEL.x, INIT_BALL_VEL.y});
  scene_add_body(state->scene, ball);
}

void make_walls(state_t *state) {
  size_t wall_width = 1;

  vector_t left = {XMIN - 1, YMAX / 2};
  body_t *left_wall = make_rect(wall_width, YMAX);
  body_set_centroid(left_wall, left);
  body_set_mass(left_wall, INFINITY);
  scene_add_body(state->scene, left_wall);

  vector_t right = {XMAX + 1, YMAX / 2};
  body_t *right_wall = make_rect(wall_width, YMAX);
  body_set_centroid(right_wall, right);
  body_set_mass(right_wall, INFINITY);
  scene_add_body(state->scene, right_wall);

  vector_t top = {XMAX / 2, YMAX + 1};
  body_t *top_wall = make_rect(YMAX, wall_width);
  body_set_centroid(top_wall, top);
  body_set_mass(top_wall, INFINITY);
  scene_add_body(state->scene, top_wall);
}

void check_player(scene_t *scene) {
  body_t *player = scene_get_body(scene, 0);
  if (body_get_centroid(player).x >= (XMAX - PLAYER_WIDTH)) {
    vector_t new_pos = body_get_centroid(player);
    new_pos.x -= SMALL_SHIFT;
    body_set_centroid(player, new_pos);
    body_set_velocity(player, VEC_ZERO);
  } else if (body_get_centroid(player).x <= (XMIN + PLAYER_WIDTH)) {
    vector_t new_pos = body_get_centroid(player);
    new_pos.x += SMALL_SHIFT;
    body_set_centroid(player, new_pos);
    body_set_velocity(player, VEC_ZERO);
  }
}

body_t *make_end_screen() {
  list_t *end = list_init(RECT_SIDES, free);
  vector_t *vec = malloc(sizeof(*vec));
  *vec = (vector_t){XMAX, YMAX};
  list_add(end, vec);
  vec = malloc(sizeof(*vec));
  *vec = (vector_t){XMIN, YMAX};
  list_add(end, vec);
  vec = malloc(sizeof(*vec));
  *vec = (vector_t){XMIN, YMIN};
  list_add(end, vec);
  vec = malloc(sizeof(*vec));
  *vec = (vector_t){XMAX, YMIN};
  list_add(end, vec);
  return body_init(end, INFINITY, (rgb_color_t){0.0, 1.0, 0.0});
}

void check_game_over(scene_t *scene) {
  size_t num_bodies = scene_bodies(scene);
  if (num_bodies <= 5) {
    scene_add_body(scene, make_end_screen());
    sdl_render_scene(scene);
    exit(0);
  }
}

bool ball_past_screen(scene_t *scene, body_t *ball) {
  if (body_get_centroid(ball).y < 0.0) {
    body_remove(ball);
    return true;
  }
  return false;
}

state_t *emscripten_init() {
  vector_t min = {XMIN, YMIN};
  vector_t max = {XMAX, YMAX};
  sdl_init(min, max);

  state_t *state = malloc(sizeof(state_t));
  state->scene = scene_init();

  state->total_time = 0.0;
  state->num_blocks = 0;
  vector_t pos = {(XMIN + XMAX) / 2, (YMIN + YMAX) / 2};
  state->start = pos;

  make_player(state);
  make_all_rects(state);
  make_walls(state);
  make_ball(state);

  body_t *ball = scene_get_body(state->scene, scene_bodies(state->scene) - 1);
  create_physics_collision(state->scene, ELASTICITY, ball,
                           scene_get_body(state->scene, 0));
  for (size_t i = 1; i < scene_bodies(state->scene) - 4; i++) {
    create_physics_collision(state->scene, ELASTICITY, ball,
                             scene_get_body(state->scene, i));
    create_color_change(state->scene, scene_get_body(state->scene, i), ball);
  }
  for (size_t i = scene_bodies(state->scene) - 4;
       i < scene_bodies(state->scene) - 1; i++) {
    create_physics_collision(state->scene, ELASTICITY, ball,
                             scene_get_body(state->scene, i));
  }

  return state;
}

void on_key(char key, key_event_type_t type, double held_time, state_t *state) {
  body_t *player = scene_get_body(state->scene, 0);
  if (type == KEY_PRESSED) {
    switch (key) {
    case RIGHT_ARROW:
      body_set_velocity(player, RVELO);
      break;
    case LEFT_ARROW:
      body_set_velocity(player, LVELO);
      break;
    }
  } else {
    body_set_velocity(player, VEC_ZERO);
  }
}

void emscripten_main(state_t *state) {
  scene_t *curr_scene = state->scene;
  sdl_on_key((key_handler_t)on_key);
  double dt = time_since_last_tick();
  state->total_time += dt;

  check_player(curr_scene);

  if (ball_past_screen(
          state->scene,
          scene_get_body(state->scene, scene_bodies(state->scene) - 1))) {
    for (size_t i = 0; i < scene_bodies(state->scene); i++) {
      body_remove(scene_get_body(state->scene, i));
    }
    scene_tick(curr_scene, dt);

    make_player(state);
    make_all_rects(state);
    make_walls(state);
    make_ball(state);

    body_t *ball = scene_get_body(state->scene, scene_bodies(state->scene) - 1);
    create_physics_collision(state->scene, ELASTICITY, ball,
                             scene_get_body(state->scene, 0));
    for (size_t i = 1; i < scene_bodies(state->scene) - 4; i++) {
      create_physics_collision(state->scene, ELASTICITY, ball,
                               scene_get_body(state->scene, i));
      create_color_change(state->scene, scene_get_body(state->scene, i), ball);
    }
    for (size_t i = scene_bodies(state->scene) - 4;
         i < scene_bodies(state->scene) - 1; i++) {
      create_physics_collision(state->scene, ELASTICITY, ball,
                               scene_get_body(state->scene, i));
    }
  }
  check_game_over(curr_scene);
  scene_tick(curr_scene, dt);
  sdl_render_scene(curr_scene);
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}