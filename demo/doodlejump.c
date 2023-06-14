#include "body.h"
#include "collision.h"
#include "forces.h"
#include "list.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "state.h"
#include "text.h"
#include "vec_list.h"
#include "vector.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

double const BLOCK_MASS = INFINITY;
size_t const COLOR_SIZE = 11;
size_t const RECT_SIDES = 4;
double const XMIN = 0.0;
double const XMAX = 1000.0;
double const YMIN = 0.0;
double const YMAX = 750.0;
double const PLAT_WIDTH = 40;
double const PLAT_HEIGHT = 10;
double const PLAYER_WIDTH = 23;
double const PLAYER_HEIGHT = -10;
size_t const PLAYER_LIST_SIZE = 6;
rgb_color_t const PLAT_COLOR = {0.55, 0.9, 0.55};
rgb_color_t const TRAP_COLOR = {0.58, 0.29, 0};
rgb_color_t const SPRING_PLAT_COLOR = {0.57, 0.29, 0};
rgb_color_t const MOVING_COLOR = {0.67, 0.84, 0.9};
rgb_color_t const PLAYER_COLOR = {0.0, 0.0, 1.0};
rgb_color_t const BALL_COLOR = {0.0, 0.0, 0.0};
rgb_color_t const PINK = (rgb_color_t){1.0, 0.8, 0.9};
rgb_color_t const PINK2 = (rgb_color_t){1.0, 0.8, 0.91};
rgb_color_t const BEIGE = (rgb_color_t){0.96, 0.91, 0.86};
// rgb_color_t const WHITE = (rgb_color_t){1, 1, 1};
rgb_color_t const WHITE = (rgb_color_t){0.5, 0.5, 0.5};
size_t const NUM_PLATS = 20;
vector_t const BOTTOM_LEFT = (vector_t){0.0, 0.0};
vector_t const TOP_RIGHT = (vector_t){1000.0, 750.0};
vector_t const TOP_LEFT = (vector_t){0.0, 750.0};
vector_t const TOP_TOP_RIGHT = (vector_t){1000.0, 900.0};
vector_t const RIGHT_MOVING_PLAT_SPEED = (vector_t){40.0, 0.0};
vector_t const LEFT_MOVING_PLAT_SPEED = (vector_t){-40.0, 0.0};
double const ACCELERATION = -400;
double const FRICTION = 1;
double const RL_VELO = 300;
vector_t const START_VELO = {0, 450};
vector_t const SPRING_VELO = {0, 700};
double const PLAYER_Y_LEVEL_1 = 350.0;
double const PLAYER_Y_LEVEL_2 = 650.0;
double const INIT_SPAWN_RATE = 2.0;
double const SPAWN_RATE_1 = 0.25;
double const SPAWN_RATE_2 = 0.15;
double const SPAWN_RATE_3 = 0.05;
double const BIG_BALL_RADIUS = 30;
vector_t const BALL_SHIFT = {0.0, -3.0};
size_t const NUM_POINTS = 100;
const double THETA = 4 * M_PI / 3;
size_t const HEART_RADIUS = 10;
vector_t const CENTER = (vector_t){500, 375};
const SDL_Color BLACK = {0.5, 0.5, 0.5};

#define PLAYER_PATH "images/doodle.png"
#define HOLE_PATH ""
#define FONT_PATH "images/DoodleJump.ttf"

rgb_color_t const color_array[COLOR_SIZE] = {
    {1.0, 0.0, 0.0}, {0.9, 0.4, 0.0}, {1.0, 1.0, 0.0}, {0.5, 0.9, 0.5},
    {0.0, 1.0, 0.0}, {0.5, 1.0, 1.0}, {0.0, 0.0, 1.0}, {0.6, 0.1, 1.0},
    {1.0, 0.2, 0.6}, {1.0, 0.8, 0.9}, {0.8, 0.8, 0.8}};

typedef struct state {
  scene_t *scene;
  size_t num_plats;
  double total_time;
  double spawn_time;
  vector_t start;
  double next_y_level;
  vector_t time_shift;
  double spawn_rate;
  size_t num_lives;
  size_t total_points;
  bool insta_create_ball;
  bool insta_create_heart;
  scene_t *start_scene;
  scene_t *end_scene;
  double screen;
  text_t text;
} state_t;

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

void make_player(state_t *state) {
  list_t *l = list_init(PLAYER_LIST_SIZE, free);
  vector_t *vec = malloc(sizeof(vector_t));

  *vec = (vector_t){PLAYER_WIDTH, 0};
  list_add(l, vec);
  vec = malloc(sizeof(vector_t));

  *vec = (vector_t){(-1) * PLAYER_WIDTH, 0};
  list_add(l, vec);
  vec = malloc(sizeof(vector_t));

  *vec = (vector_t){(-1) * PLAYER_WIDTH, PLAYER_HEIGHT};
  list_add(l, vec);
  vec = malloc(sizeof(vector_t));

  *vec = (vector_t){PLAYER_WIDTH, PLAYER_HEIGHT};
  list_add(l, vec);

  body_t *player = body_init_with_info(l, BLOCK_MASS, color_array[0],
                                       PLAYER_PATH, NULL, NULL, NULL);

  body_set_color(player, PLAYER_COLOR);
  body_set_centroid(player, (vector_t){XMAX / 2.0, YMAX / 3.0});
  scene_add_body(state->scene, player);
}

vector_t get_random_point(vector_t min, vector_t max) {
  size_t xmin = (size_t)min.x;
  size_t ymin = (size_t)min.y;
  size_t xmax = (size_t)max.x;
  size_t ymax = (size_t)max.y;

  double xrand = rand() % (xmax - xmin);
  double yrand = rand() % (ymax - ymin);
  xrand += xmin;
  yrand += ymin;

  if (xrand <= PLAT_WIDTH) {
    xrand += PLAT_WIDTH;
  }
  if (xrand >= xmax - PLAT_WIDTH) {
    xrand -= PLAT_WIDTH;
  }
  if (yrand <= PLAT_HEIGHT) {
    yrand += PLAT_HEIGHT;
  }

  vector_t random_point = (vector_t){xrand, yrand};
  return random_point;
}

body_t *make_spring_rect(double width, double height) {
  list_t *rect = list_init(RECT_SIDES, free);
  vector_t *vec = malloc(sizeof(vector_t));
  *vec = (vector_t){width, height + 20};
  list_add(rect, vec);
  vec = malloc(sizeof(vector_t));
  *vec = (vector_t){width - 20, height + 20};
  list_add(rect, vec);
  vec = malloc(sizeof(vector_t));
  *vec = (vector_t){width - 20, height};
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

void *make_spring_plat() {
  body_t *platform = make_spring_rect(PLAT_WIDTH, PLAT_HEIGHT / 2);
  body_set_color(platform, SPRING_PLAT_COLOR);
  body_add_image(platform, "images/spring_platform.png");
}

body_t *make_regular_plat() {
  body_t *platform = make_rect(PLAT_WIDTH, PLAT_HEIGHT);
  body_set_color(platform, PLAT_COLOR);
  body_add_image(platform, "images/reg_platform.png");
  return platform;
}

body_t *make_trap_plat() {
  body_t *platform = make_rect(PLAT_WIDTH, PLAT_HEIGHT);
  body_set_color(platform, TRAP_COLOR);
  body_add_image(platform, "images/trap_platform.png");
  return platform;
}

body_t *make_moving_plat() {
  body_t *platform = make_rect(PLAT_WIDTH, PLAT_HEIGHT);
  body_set_color(platform, MOVING_COLOR);
  body_add_image(platform, "images/moving_platform.png");
  return platform;
}

void make_all_plats(state_t *state) {
  body_t *platform = make_regular_plat();
  body_set_centroid(platform, (vector_t){XMAX / 2.0, YMAX / 10.0});
  scene_add_body(state->scene, platform);
  state->num_plats++;

  double curr_y = YMAX / 10.0;

  while (curr_y <= YMAX) {
    curr_y += 45.0;
    double rand_plat_num = rand() % 10;
    vector_t rand_point = get_random_point((vector_t){XMIN, curr_y - 1.0},
                                           (vector_t){XMAX, curr_y});

    body_t *next_platform;
    if (rand_plat_num < 4.0) {
      next_platform = make_regular_plat();
    } else if (rand_plat_num < 5.0) {
      next_platform = make_spring_plat();
    } else if (rand_plat_num < 8.0) {
      next_platform = make_moving_plat();
      body_set_velocity(next_platform, RIGHT_MOVING_PLAT_SPEED);
    } else {
      next_platform = make_trap_plat();
    }
    body_set_centroid(next_platform, rand_point);
    scene_add_body(state->scene, next_platform);
    state->num_plats++;
  }
}

body_t *make_big_ball() {
  list_t *ball_points = list_init(16, free);
  vector_t rand_point =
      get_random_point((vector_t){0.0, 749.0}, (vector_t){1000.0, 751.0});

  double const rotation_angle = 2 * M_PI / 15;

  for (size_t i = 0; i < 15; i++) {
    double angle = i * rotation_angle;

    vector_t *vec = malloc(sizeof(vector_t));
    vec->x = BIG_BALL_RADIUS * cos(angle);
    vec->y = BIG_BALL_RADIUS * sin(angle);
    *vec = vec_add(*vec, rand_point);

    list_add(ball_points, vec);
  }

  body_t *big_ball = body_init(ball_points, INFINITY, BALL_COLOR);
  body_add_image(big_ball, "images/monster.png");
  return big_ball;
}

body_t *make_heart(state_t *state) {
  list_t *heart_coords = list_init(NUM_POINTS, free);
  vector_t rand_point = {40, 40};

  double const rotation_angle = 2 * M_PI / NUM_POINTS;

  for (size_t i = 0; i < NUM_POINTS / 2; i++) {
    double angle = i * rotation_angle;

    vector_t *vec = malloc(sizeof(vector_t));
    vec->x = HEART_RADIUS * cos(angle);
    vec->y = HEART_RADIUS * sin(angle);
    *vec = vec_add(*vec, rand_point);

    list_add(heart_coords, vec);
  }
  for (size_t i = 0; i < NUM_POINTS / 2; i++) {
    double angle = i * rotation_angle;

    vector_t *vec = malloc(sizeof(vector_t));
    vec->x = HEART_RADIUS * cos(angle) + 2 * HEART_RADIUS;
    vec->y = HEART_RADIUS * sin(angle);
    *vec = vec_add(*vec, rand_point);

    list_add(heart_coords, vec);
  }
  vector_t *las_pt = malloc(sizeof(vector_t));
  las_pt->x = rand_point.x + 3 * HEART_RADIUS;
  las_pt->y = rand_point.y;
  list_add(heart_coords, las_pt);

  vector_t *last_pt = malloc(sizeof(vector_t));
  last_pt->x = rand_point.x + HEART_RADIUS;
  last_pt->y = rand_point.y - 2 * HEART_RADIUS;
  list_add(heart_coords, last_pt);

  vector_t *lat_pt = malloc(sizeof(vector_t));
  lat_pt->x = rand_point.x - HEART_RADIUS;
  lat_pt->y = rand_point.y;
  list_add(heart_coords, lat_pt);

  body_t *heart = body_init(heart_coords, INFINITY, PINK);
  return heart;
}

void make_all_hearts(state_t *state) {
  body_t *heart1 = make_heart(state);
  body_set_centroid(heart1, (vector_t){30, 1});
  scene_add_body(state->scene, heart1);
  state->num_lives++;

  body_t *heart2 = make_heart(state);
  body_set_centroid(heart2, (vector_t){75, 1});
  scene_add_body(state->scene, heart2);
  state->num_lives++;

  body_t *heart3 = make_heart(state);
  body_set_centroid(heart3, (vector_t){120, 1});
  scene_add_body(state->scene, heart3);
  state->num_lives++;
}

void check_moving_plats(state_t *state) {
  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    body_t *curr_body = scene_get_body(state->scene, i);
    if (equals(body_get_color(curr_body), MOVING_COLOR)) {
      if (body_get_centroid(curr_body).x >= XMAX - PLAT_WIDTH) {
        body_set_velocity(curr_body, LEFT_MOVING_PLAT_SPEED);
      } else if (body_get_centroid(curr_body).x <= XMIN + PLAT_WIDTH) {
        body_set_velocity(curr_body, RIGHT_MOVING_PLAT_SPEED);
      }
    }
  }
}

void check_bounce(state_t *state) {
  body_t *player = scene_get_body(state->scene, 0);

  size_t counter = 1 + state->num_lives;
  while (counter < scene_bodies(state->scene)) {
    body_t *curr = scene_get_body(state->scene, counter);
    collision_info_t col =
        find_collision(body_get_shape(curr), body_get_shape(player));
    if (col.collided && !equals(body_get_color(curr), BALL_COLOR) &&
        !equals(body_get_color(curr), PINK2) &&
        !equals(body_get_color(curr), WHITE)) {
      if (body_get_velocity(player).y < 0.0) {
        if (equals(body_get_color(curr), SPRING_PLAT_COLOR)) {
          body_set_velocity(player, SPRING_VELO);

          Mix_Chunk *wave = Mix_LoadWAV("images/spring_jump.wav");
          Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);
          Mix_PlayChannel(-1, wave, 0);
        } else if (!equals(body_get_color(curr), TRAP_COLOR)) {
          body_set_velocity(player, START_VELO);

          Mix_Chunk *wave = Mix_LoadWAV("images/reg_jump.wav");
          Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);
          Mix_PlayChannel(-1, wave, 0);
        } else {
          body_remove(curr);
          state->insta_create_ball = false;
          state->num_plats--;

          Mix_Chunk *wave = Mix_LoadWAV("images/trap_jump.wav");
          Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);
          Mix_PlayChannel(-1, wave, 0);
        }
      }
    }
    counter++;
  }
}

body_t *get_first_non_trap_platform(state_t *state) {
  for (size_t i = 4; i < scene_bodies(state->scene); i++) {
    body_t *curr = scene_get_body(state->scene, i);
    rgb_color_t curr_color = body_get_color(curr);
    if (equals(curr_color, PLAT_COLOR) || equals(curr_color, MOVING_COLOR)) {
      return curr;
    }
  }
}

bool check_death(state_t *state) {
  // Checks if it collides with a falling ball
  body_t *player = scene_get_body(state->scene, 0);
  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    body_t *curr = scene_get_body(state->scene, i);
    collision_info_t col =
        find_collision(body_get_shape(curr), body_get_shape(player));
    if (equals(body_get_color(curr), BALL_COLOR) && col.collided) {
      body_remove(curr);

      Mix_Chunk *wave = Mix_LoadWAV("images/collide_monster.wav");
      Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);
      Mix_PlayChannel(-1, wave, 0);
      if (state->num_lives == 1) {
        body_set_color(scene_get_body(state->scene, state->num_lives + 1),
                       WHITE);
        return true;
      } else {
        state->num_lives--;
        body_set_centroid(
            player, body_get_centroid(get_first_non_trap_platform(state)));
        body_set_color(scene_get_body(state->scene, state->num_lives + 1),
                       WHITE);
      }
    } else if (equals(body_get_color(curr), PINK2) && col.collided) {
      body_remove(curr);
      state->insta_create_heart = false;
      if (state->num_lives < 3) {
        Mix_Chunk *wave = Mix_LoadWAV("images/collide_heart.wav");
        Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);
        Mix_PlayChannel(-1, wave, 0);
      }
      if (state->num_lives == 1) {
        state->num_lives++;
        body_set_color(scene_get_body(state->scene, 2), PINK);
      } else if (state->num_lives == 2) {
        state->num_lives++;
        body_set_color(scene_get_body(state->scene, 3), PINK);
      }
    }
  }

  // Checks if it falls off the screen
  if (body_get_centroid(player).y <= 0.0) {
    Mix_Chunk *wave = Mix_LoadWAV("images/fall_death.wav");
    Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);
    Mix_PlayChannel(-1, wave, 0);

    if (state->num_lives == 1) {
      body_set_color(scene_get_body(state->scene, 1), WHITE);
      return true;
    } else {
      state->num_lives--;
      body_set_centroid(player,
                        body_get_centroid(get_first_non_trap_platform(state)));
      body_set_color(scene_get_body(state->scene, state->num_lives + 1), WHITE);
    }
  }
  return false;
}

void move_falling_objects(state_t *state) {
  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    body_t *curr_body = scene_get_body(state->scene, i);
    if (equals(body_get_color(curr_body), BALL_COLOR) ||
        equals(body_get_color(curr_body), PINK2)) {
      vector_t pos = body_get_centroid(curr_body);
      vector_t n_pos = vec_add(pos, BALL_SHIFT);
      body_set_centroid(curr_body, n_pos);
    }
  }
}

void add_new_platform(state_t *state) {
  double rand_plat_num = rand() % 10;
  vector_t rand_point =
      get_random_point((vector_t){XMIN, state->next_y_level - 1.0},
                       (vector_t){XMAX, state->next_y_level});
  vector_t rand_point2 =
      get_random_point((vector_t){XMIN, state->next_y_level - 1.0},
                       (vector_t){XMAX, state->next_y_level});
  if (fabs(rand_point2.x - rand_point.x) < PLAT_WIDTH) {
    if (rand_point2.x < 2 * PLAT_WIDTH) {
      rand_point2.x += 2 * PLAT_WIDTH;
    } else {
      rand_point2.x -= 2 * PLAT_WIDTH;
    }
  }

  state->next_y_level += 10.0;
  body_t *next_platform;
  if (rand_plat_num < 3.0) {
    next_platform = make_regular_plat();
  } else if (rand_plat_num < 5.0) {
    next_platform = make_spring_plat();
  } else if (rand_plat_num < 8.0) {
    next_platform = make_moving_plat();
    body_set_velocity(next_platform, RIGHT_MOVING_PLAT_SPEED);
  } else {
    next_platform = make_trap_plat();
    body_t *green_plat = make_regular_plat();
    body_set_centroid(green_plat, rand_point2);
    scene_add_body(state->scene, green_plat);
    state->num_plats++;
  }

  body_set_centroid(next_platform, rand_point);
  scene_add_body(state->scene, next_platform);
  state->num_plats++;
}

void update_platforms(state_t *state) {
  body_t *player = scene_get_body(state->scene, 0);
  vector_t pos = body_get_centroid(player);
  vector_t n_pos = vec_add(pos, state->time_shift);
  body_set_centroid(player, n_pos);

  for (size_t i = 4; i < scene_bodies(state->scene); i++) {
    body_t *curr = scene_get_body(state->scene, i);
    vector_t pos = body_get_centroid(curr);
    vector_t n_pos = vec_add(pos, state->time_shift);
    body_set_centroid(curr, n_pos);
  }
}

void update_hearts(state_t *state) {
  for (size_t i = 1; i < 4; i++) {
    body_t *curr = scene_get_body(state->scene, i);
    vector_t pos = body_get_centroid(curr);
    vector_t n_pos = vec_subtract(pos, state->time_shift);
    body_set_centroid(curr, pos);
  }
}

void player_wrap(scene_t *scene) {
  body_t *player = scene_get_body(scene, 0);
  vector_t center = body_get_centroid(player);
  double vec_x = center.x;
  double vec_y = center.y;

  if (vec_x <= XMIN) {
    vector_t new_pos = (vector_t){XMAX, vec_y};
    body_set_centroid(player, new_pos);
  } else if (vec_x >= XMAX) {
    vector_t new_pos = (vector_t){XMIN, vec_y};
    body_set_centroid(player, new_pos);
  }
}

void create_start_screen(state_t *state) {
  state->start_scene = scene_init();
  body_t *body = make_rect(500.0, 375.0);
  body_set_color(body, BEIGE);
  body_set_centroid(body, CENTER);
  scene_add_body(state->start_scene, body);

  body_t *phrase = make_rect(10, 10);
  TTF_Font *font1 = TTF_OpenFont(FONT_PATH, 48);
  vector_t loc = {340, 570};

  char *title = "Doodle Jump";
  text_t *text = text_init(title, font1, BLACK, loc, (free_func_t)free);
  body_add_text(phrase, text);
  scene_add_body(state->start_scene, phrase);

  body_t *phrase1 = make_rect(10, 10);
  TTF_Font *font2 = TTF_OpenFont(FONT_PATH, 36);
  vector_t loc1 = {325, 320};

  char *title1 = "Press Up to Start";
  text_t *text1 = text_init(title1, font2, BLACK, loc1, (free_func_t)free);
  body_add_text(phrase1, text1);
  scene_add_body(state->start_scene, phrase1);

  sdl_render_scene(state->start_scene);
}

void create_end_screen(state_t *state) {
  state->end_scene = scene_init();
  body_t *body = make_rect(500.0, 375.0);
  body_set_color(body, BEIGE);
  body_set_centroid(body, CENTER);
  scene_add_body(state->end_scene, body);

  body_t *phrase = make_rect(10, 10);
  TTF_Font *font = TTF_OpenFont(FONT_PATH, 48);
  vector_t loc = {350, 550};
  char *title = "Game Over";
  text_t *text = text_init(title, font, BLACK, loc, (free_func_t)free);
  body_add_text(phrase, text);
  scene_add_body(state->end_scene, phrase);

  body_t *phrase1 = make_rect(10, 10);
  TTF_Font *font1 = TTF_OpenFont(FONT_PATH, 36);
  vector_t loc1 = {275, 340};
  char *title1 = "Press Up to Play Again";
  text_t *text1 = text_init(title1, font1, BLACK, loc1, (free_func_t)free);
  body_add_text(phrase1, text1);
  scene_add_body(state->end_scene, phrase1);

  body_t *phrase2 = make_rect(10, 10);
  vector_t loc2 = {320, 200};
  char *title2 = "Total Points: ";
  text_t *text2 = text_init(title2, font1, BLACK, loc2, (free_func_t)free);
  body_add_text(phrase2, text2);
  scene_add_body(state->end_scene, phrase2);

  body_t *phrase3 = make_rect(10, 10);
  vector_t loc3 = {630, 200};
  char *title3;
  snprintf(title3, sizeof title3, "%zu", state->total_points);
  text_t *text3 = text_init(title3, font1, BLACK, loc3, (free_func_t)free);
  body_add_text(phrase3, text3);
  scene_add_body(state->end_scene, phrase3);

  sdl_render_scene(state->end_scene);
}

void add_score(state_t *state) {
  body_t *phrase = make_rect(10, 10);
  TTF_Font *font1 = TTF_OpenFont(FONT_PATH, 48);
  vector_t loc = {250, 250};

  char *title = "state->total_points";
  text_t *text = text_init(title, font1, BLACK, loc, (free_func_t)free);
  body_add_text(phrase, text);
  scene_add_body(state->scene, phrase);
  sdl_render_scene(state->scene);
}

state_t *emscripten_init() {
  vector_t min = {XMIN, YMIN};
  vector_t max = {XMAX, YMAX};
  sdl_init(min, max);

  state_t *state = malloc(sizeof(state_t));
  state->scene = scene_init();

  state->total_time = 0.0;
  state->spawn_time = 0.0;
  state->next_y_level = YMAX + 15.0;
  vector_t pos = {(XMIN + XMAX) / 2, (YMIN + YMAX) / 2};
  state->start = pos;
  state->num_plats = 0;
  state->time_shift = VEC_ZERO;
  state->spawn_rate = INIT_SPAWN_RATE;
  state->num_lives = 0;
  state->total_points = 0;
  state->insta_create_ball = false;
  state->insta_create_heart = false;
  state->screen = 1.0;

  make_player(state);
  make_all_hearts(state);
  //   add_score(state);
  make_all_plats(state);

  return state;
}

void free_objects(state_t *state) {
  for (size_t i = 1; i < scene_bodies(state->scene); i++) {
    body_t *curr_object = scene_get_body(state->scene, i);
    vector_t curr_pos = body_get_centroid(curr_object);
    rgb_color_t curr_color = body_get_color(curr_object);

    if (curr_pos.y <= YMIN) {
      if (equals(curr_color, BALL_COLOR)) {
        state->insta_create_ball = false; // a new ball can now be dropped
      } else if (equals(curr_color, PINK2)) {
        state->insta_create_heart = false;
      } else {
        state->num_plats--;
        state->total_points++;
      }
      body_remove(curr_object); // remove the body regardless
    }
  }
}

void on_key(char key, key_event_type_t type, double held_time, state_t *state) {
  body_t *player = scene_get_body(state->scene, 0);
  double dt = time_since_last_tick();

  vector_t c_velo = body_get_velocity(player);
  vector_t nr_velo = (vector_t){RL_VELO, c_velo.y + 4 * ACCELERATION * dt};
  vector_t nl_velo = (vector_t){-RL_VELO, c_velo.y + 4 * ACCELERATION * dt};
  vector_t no_velo = (vector_t){0, c_velo.y + ACCELERATION * dt};

  if (type == KEY_PRESSED) {
    switch (key) {
    case RIGHT_ARROW:
      body_set_velocity(player, nr_velo);
      break;
    case LEFT_ARROW:
      body_set_velocity(player, nl_velo);
      break;
    case UP_ARROW:
      if (state->screen == 1.0) {
        state->screen = 2.0;
        Mix_Chunk *wave = Mix_LoadWAV("images/start.wav");
        Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);
        Mix_PlayChannel(-1, wave, 0);
      }
      if (state->screen == 3.0) {
        state->screen = 2.0;
        state->num_lives = 3;
        state->total_points = 0;
        for (size_t i = 1; i < 4; i++) {
          body_set_color(scene_get_body(state->scene, i), PINK);
        }
        Mix_Chunk *wave = Mix_LoadWAV("images/start.wav");
        Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);
        Mix_PlayChannel(-1, wave, 0);
      }
      break;
    }
  } else {
    body_set_velocity(player, no_velo);
  }
}

void emscripten_main(state_t *state) {
  sdl_on_key((key_handler_t)on_key);
  double dt = time_since_last_tick();
  if (state->screen == 1.0) {
    create_start_screen(state);
  } else if (state->screen == 2.0) {
    scene_t *curr_scene = state->scene;
    // double dt = time_since_last_tick();
    state->total_time += dt;
    state->spawn_time += dt;
    body_t *player = scene_get_body(curr_scene, 0);

    vector_t curr_velo = body_get_velocity(player);

    double acc = ACCELERATION;
    if (body_get_centroid(player).y > YMAX) {
      acc *= 5;
    }

    vector_t new_vel = (vector_t){curr_velo.x, curr_velo.y + acc * dt};
    body_set_velocity(player, new_vel);

    if (body_get_centroid(player).y > YMAX) {
      state->time_shift = (vector_t){0, -15.0};
      state->spawn_rate = SPAWN_RATE_3;
    } else if (body_get_centroid(player).y > PLAYER_Y_LEVEL_2) {
      state->time_shift = (vector_t){0, -10.0};
      state->spawn_rate = SPAWN_RATE_2;
    } else if (body_get_centroid(player).y > PLAYER_Y_LEVEL_1) {
      state->time_shift = (vector_t){0, -5.0};
      state->spawn_rate = SPAWN_RATE_1;
    } else {
      state->time_shift = VEC_ZERO;
      state->spawn_rate = INIT_SPAWN_RATE;
    }

    if (state->total_points >= 10 && state->total_points % 5 == 0 &&
        !state->insta_create_ball) {
      body_t *ball = make_big_ball();
      scene_add_body(curr_scene, ball);
      state->insta_create_ball = true;
    }

    if (state->total_points >= 20 && state->total_points % 10 == 2 &&
        !state->insta_create_heart) {
      body_t *heart = make_heart(state);
      body_set_color(heart, PINK2);
      vector_t rand_pt = get_random_point((vector_t){XMIN, YMAX - 1},
                                          (vector_t){XMAX, YMAX + 1});
      body_set_centroid(heart, rand_pt);
      scene_add_body(curr_scene, heart);
      state->insta_create_heart = true;
    }

    // add_score(state);
    check_bounce(state);
    update_platforms(state);
    update_hearts(state);
    move_falling_objects(state);
    player_wrap(state->scene);
    check_moving_plats(state);
    if (state->spawn_time >= state->spawn_rate) {
      add_new_platform(state);
      state->spawn_time = 0.0;
    }
    free_objects(state);
    bool end = check_death(state);

    if (!end) {
      scene_tick(state->scene, dt);
      sdl_render_scene(curr_scene);
    } else {
      state->screen = 3.0;
    }
  } else if (state->screen == 3.0) {
    create_end_screen(state);
  }
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  scene_free(state->start_scene);
  scene_free(state->end_scene);
  free(state);
}