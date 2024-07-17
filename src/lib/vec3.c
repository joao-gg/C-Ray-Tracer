#include <math.h>

#include "vec3.h"

Vec3_t vec3_add(Vec3_t a, Vec3_t b) {
    return Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vec3_t vec3_sub(Vec3_t a, Vec3_t b) {
    return Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vec3_t vec3_mul(Vec3_t a, Vec3_t b) {
    return Vec3(a.x * b.x, a.y * b.y, a.z * b.z);
}

Vec3_t vec3_scalar_mul(Vec3_t a, float scalar) {
    return Vec3(a.x * scalar, a.y * scalar, a.z * scalar);
}

float vec3_dot(Vec3_t a, Vec3_t b) {
    return (a.x * b.x + a.y * b.y + a.z * b.z);
}

float vec3_self_dot(Vec3_t a) {
    return (a.x * a.x + a.y * a.y + a.z * a.z);
}

Vec3_t vec3_norm(Vec3_t a) {
    float magnitude = sqrtf(vec3_self_dot(a));

    if (magnitude == 0) {
        return Vec3(0, 0, 0);
    }
    return Vec3(a.x / magnitude, a.y / magnitude, a.z / magnitude);
}

float vec3_magnitude(Vec3_t a) {
    return sqrtf(vec3_self_dot(a));
}
