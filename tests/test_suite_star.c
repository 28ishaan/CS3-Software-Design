#include "star.h"
#include "test_util.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

star_t *make_star_test() {
  vector_t *start_pos = malloc(sizeof(vector_t));
  start_pos->x = 10.0;
  start_pos->y = 10.0;
  star_t *star =
      star_init(2, start_pos, 5, (rgb_color_t){1.0, 0.0, 0.0}, 10.0, 10.0);
  free(start_pos);
  return star;
}

void test_star_get_velocity() {
  star_t *star = make_star_test();
  assert(get_star_velocity(star).x == 10.0);
  assert(get_star_velocity(star).y == 10.0);
  free_star(star);
}

void test_star_get_color() {
  star_t *star = make_star_test();
  assert(0.0 == 0.0);
  free_star(star);
}

void test_star_set_points() {
  star_t *star = make_star_test();
  assert(0.0 == 0.0);
  free_star(star);
}

void test_star_set_velocity() {
  star_t *star = make_star_test();
  assert(0.0 == 0.0);
  free_star(star);
}

int main(int argc, char *argv[]) {
  // Run all tests? True if there are no command-line arguments
  bool all_tests = argc == 1;
  // Read test name from file
  char testname[100];
  if (!all_tests) {
    read_testname(argv[1], testname, sizeof(testname));
  }

  // DO_TEST(test_star_get_points)
  DO_TEST(test_star_get_velocity)
  DO_TEST(test_star_get_color)
  DO_TEST(test_star_set_points)
  DO_TEST(test_star_set_velocity)

  puts("star_test PASS");
}
