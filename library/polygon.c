#include "polygon.h"
#include <math.h>
#include <stdlib.h>

double polygon_area(list_t *polygon) {
  double area = 0.0;
  size_t n = list_size(polygon);
  for (size_t i = 0; i < n; i++) {
    size_t j = (i + 1) % n;

    vector_t *v1 = list_get(polygon, i);
    vector_t *v2 = list_get(polygon, j);

    area += v1->x * v2->y - v2->x * v1->y;
  }
  return 0.5 * area;
}

vector_t polygon_centroid(list_t *polygon) {
  double area = 0.0;
  size_t n = list_size(polygon);
  double cx_sum = 0.0;
  double cy_sum = 0.0;

  for (size_t i = 0; i < n; i++) {
    size_t j = (i + 1) % n;

    vector_t *v1 = list_get(polygon, i);
    vector_t *v2 = list_get(polygon, j);

    double cross = v1->x * v2->y - v2->x * v1->y;
    area += cross;
    cx_sum += (v1->x + v2->x) * cross;
    cy_sum += (v1->y + v2->y) * cross;
  }

  area /= 2.0;

  // vector_t *centroid = list_get(polygon, 0);
  // centroid->x = cx_sum / (6.0 * area);
  // centroid->y = cy_sum / (6.0 * area);

  vector_t output = {cx_sum / (6.0 * area), cy_sum / (6.0 * area)};

  return output;
}

void polygon_translate(list_t *polygon, vector_t translation) {
  size_t n = list_size(polygon);
  for (size_t i = 0; i < n; i++) {
    vector_t *add = list_get(polygon, i);
    vector_t new_add = vec_add(*add, translation);
    add->x = new_add.x;
    add->y = new_add.y;
  }
}

void polygon_rotate(list_t *polygon, double angle, vector_t point) {
  size_t n = list_size(polygon);
  for (size_t i = 0; i < n; i++) {
    vector_t *vector = list_get(polygon, i);
    vector_t subtract = vec_subtract(*vector, point);
    vector_t rotate = vec_rotate(subtract, angle);
    vector_t add = vec_add(rotate, point);

    vector->x = add.x;
    vector->y = add.y;
  }
}
