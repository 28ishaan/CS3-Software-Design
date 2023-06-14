#include "collision.h"
#include "list.h"
#include "vector.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void project_shape_on_axis(list_t *shape, vector_t *axis, double *min,
                           double *max) {
  *min = vec_dot(*(vector_t *)list_get(shape, 0), *axis);
  *max = *min;

  for (size_t i = 1; i < list_size(shape); i++) {
    double projection = vec_dot(*(vector_t *)list_get(shape, i), *axis);
    if (projection < *min) {
      *min = projection;
    } else if (projection > *max) {
      *max = projection;
    }
  }
}

collision_info_t find_collision(list_t *shape1, list_t *shape2) {
  vector_t *axis = malloc(sizeof(vector_t));
  vector_t *minaxis = malloc(sizeof(vector_t));
  collision_info_t new_collision_info;
  double min_overlap = DBL_MAX;
  for (size_t i = 0; i < list_size(shape1) + list_size(shape2); i++) {
    if (i < list_size(shape1)) {
      vector_t current = *(vector_t *)list_get(shape1, i);
      vector_t next =
          *(vector_t *)list_get(shape1, (i + 1) % list_size(shape1));
      axis->x = -(current.y - next.y);
      axis->y = current.x - next.x;
    } else {
      vector_t current = *(vector_t *)list_get(shape2, i - list_size(shape1));
      vector_t next =
          *(vector_t *)list_get(shape2, (i + 1) % list_size(shape2));
      axis->x = -(current.y - next.y);
      axis->y = current.x - next.x;
    }

    double axis_length = sqrt(vec_dot(*axis, *axis));
    axis->x = axis->x / axis_length;
    axis->y = axis->y / axis_length;

    double min_shape1, max_shape1, min_shape2, max_shape2;
    project_shape_on_axis(shape1, axis, &min_shape1, &max_shape1);
    project_shape_on_axis(shape2, axis, &min_shape2, &max_shape2);

    if (max_shape1 < min_shape2 || min_shape1 > max_shape2) {
      new_collision_info.collided = false;
      free(axis);
      free(minaxis);
      return new_collision_info;
    } else {
      double overlap =
          fmin(max_shape1, max_shape2) - fmax(min_shape1, min_shape2);
      if (overlap <= min_overlap) {
        min_overlap = overlap;
        *minaxis = *axis;
      }
    }
  }

  new_collision_info.axis = *minaxis;
  free(axis);
  free(minaxis);
  new_collision_info.collided = true;
  return new_collision_info;
}
