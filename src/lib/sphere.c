#include <math.h>
#include <stdbool.h>

#include "vec3.h"
#include "sphere.h"

bool intersect_sphere(const Sphere_t sphere, const Vec3_t origin,
                      const Vec3_t direction, float *dist) {
    Vec3_t origin_sphere_dist = vec3_sub(origin, sphere.pos);

    float a = vec3_self_dot(direction);
    float b = 2 * vec3_dot(direction, origin_sphere_dist);
    float c = vec3_self_dot(origin_sphere_dist) - sphere.radius * sphere.radius;

    float delta = b * b - 4 * a * c;

    if (delta >= 0) {
        float sqrt_delta = sqrtf(delta);
        float denominator = 2.0 * a;

        float t1 = (-b - sqrt_delta) / denominator;
        float t2 = (-b + sqrt_delta) / denominator;

        // ignorar interseções muito próximas
        if (t1 > 0.1 || t2 > 0.1) {
            // encontrar a menor distância de interseção
            *dist = t1 < t2 ? t1 : t2;
            return true;
        }
    }
    return false;
}