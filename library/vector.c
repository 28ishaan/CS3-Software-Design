#include <math.h>
#include <vector.h>

const vector_t VEC_ZERO = {0.0, 0.0};

vector_t vec_add(vector_t v1, vector_t v2) {
  vector_t v_add = v1;
  v_add.x += v2.x;
  v_add.y += v2.y;

  return v_add;
}

vector_t vec_subtract(vector_t v1, vector_t v2) {
  vector_t v_sub = v1;
  v_sub.x -= v2.x;
  v_sub.y -= v2.y;

  return v_sub;
}

vector_t vec_negate(vector_t v) { return vec_multiply(-1, v); }

vector_t vec_multiply(double scalar, vector_t v) {
  vector_t v_mult = v;
  v_mult.x *= scalar;
  v_mult.y *= scalar;
  return v_mult;
}

double vec_dot(vector_t v1, vector_t v2) { return v1.x * v2.x + v1.y * v2.y; }

double vec_cross(vector_t v1, vector_t v2) { return v1.x * v2.y - v1.y * v2.x; }

vector_t vec_rotate(vector_t v, double angle) {
  vector_t v_rot = v;
  v_rot.x = v.x * cos(angle) - v.y * sin(angle);
  v_rot.y = v.x * sin(angle) + v.y * cos(angle);
  return v_rot;
}

double vec_magnitude(vector_t v) { return sqrt(pow(v.x, 2) + pow(v.y, 2)); }