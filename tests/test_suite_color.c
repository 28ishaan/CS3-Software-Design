#include "color.h"
#include "test_util.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

void test_rand_color() {
  rgb_color_t color = rand_color();

  assert(get_r_color(color) >= 0.0 && get_r_color(color) <= 1.0);
  assert(get_g_color(color) >= 0.0 && get_g_color(color) <= 1.0);
  assert(get_b_color(color) >= 0.0 && get_b_color(color) <= 1.0);
}

int main(int argc, char *argv[]) {
  // Run all tests? True if there are no command-line arguments
  bool all_tests = argc == 1;
  // Read test name from file
  char testname[100];
  if (!all_tests) {
    read_testname(argv[1], testname, sizeof(testname));
  }

  DO_TEST(test_rand_color)

  puts("color_test PASS");
}