#ifndef VEC3_H
#define VEC3_H

typedef struct Vec3_s
{
    float x, y, z;
} Vec3_t;

#define Vec3(x, y, z) \
    (Vec3_t) { (x), (y), (z) }

Vec3_t vec3_add(Vec3_t a, Vec3_t b);

Vec3_t vec3_sub(Vec3_t a, Vec3_t b);

Vec3_t vec3_mul(Vec3_t a, Vec3_t b);

Vec3_t vec3_scalar_mul(Vec3_t a, float scalar);

Vec3_t vec3_norm(Vec3_t a);

float vec3_dot(Vec3_t a, Vec3_t b);

float vec3_self_dot(Vec3_t a);

float vec3_magnitude(Vec3_t a);

#endif // VEC3_H