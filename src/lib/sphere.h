#ifndef SPHERE_H
#define SPHERE_H

#include <stdbool.h>

#include "vec3.h"

typedef struct {
    Vec3_t pos, color;
    float radius;
    bool is_reflective;
} Sphere_t;

#define Sphere(pos, color, radius, is_reflective) \
    (Sphere_t) { (pos), (color), (radius), (is_reflective) }

bool intersect_sphere(const Sphere_t sphere, const Vec3_t origin,
                      const Vec3_t direction, float *dist);

#endif // SPHERE_H