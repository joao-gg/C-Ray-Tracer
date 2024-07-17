#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "lib/vec3.h"

// algumas cores para você experimentar
#define COL_RED         Vec3(1,      0,      0)
#define COL_DARK_RED    Vec3(0.4,    0,      0)
#define COL_GREEN       Vec3(0,      1,      0)
#define COL_BLUE        Vec3(0,      0,      1)
#define COL_WHITE       Vec3(1,      1,      1)
#define COL_BLACK       Vec3(0,      0,      0)
#define COL_YELLOW_LAMP Vec3(1,      0.9,    0.7)
#define COL_STELL       Vec3(0.4431, 0.4745, 0.4941)

#define DIFFUSE_INTENSITY    0.7
#define SPECULAR_INTENSITY   0.3
#define SPECULAR_EXPONENT    50.0
#define AMBIENT_INTENSITY    0.1
#define REFLECTION_INTENSITY 0.50

typedef struct {
    Vec3_t pos, color;
    float radius;
    bool is_reflective;
} Sphere_t;

#define Sphere(pos, color, radius, is_reflective) \
    (Sphere_t) { (pos), (color), (radius), (is_reflective) }

static inline bool intersect_sphere(const Sphere_t sphere, const Vec3_t origin,
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

typedef struct {
    Sphere_t *lights, *spheres;
    unsigned num_lights, num_spheres;
} SceneObjects_t;

#define SceneObjects(lights_array, spheres_array)            \
    (SceneObjects_t) {                                       \
        (lights_array), (spheres_array),                     \
            (sizeof(lights_array) / sizeof(*lights_array)),  \
            (sizeof(spheres_array) / sizeof(*spheres_array)) \
    }

typedef struct {
    unsigned width, height;
    float aspect_ratio;
    SceneObjects_t objects;
    Vec3_t bg_color;
} SceneOpts_t;

typedef struct {
    Vec3_t camera_pos;
    float yaw, pitch;
    unsigned FOV, ray_depth;
} RenderOpts_t;

static inline void __checkerboard_col(Sphere_t *sphere, Vec3_t intersection,
                                         Vec3_t col1, Vec3_t col2) {
    int checkerboard = ((int)(intersection.x + 100) + (int)(intersection.z + 100)) & 1;
    sphere->color    = (checkerboard ? col1 : col2);
}

static inline Vec3_t trace(SceneOpts_t scene_opts, const Vec3_t origin,
                           const Vec3_t direction, unsigned ray_depth) {
    float closest_intersection_dist = INFINITY;
    float intersection_dist = 0;
    long curr_sphere = -1;

    for (unsigned i = 0; i < scene_opts.objects.num_spheres; ++i) {
        bool found_sphere = intersect_sphere(scene_opts.objects.spheres[i], 
                                             origin, direction, &intersection_dist);

        if (found_sphere && (intersection_dist < closest_intersection_dist)) {
            closest_intersection_dist = intersection_dist;
            curr_sphere = i;
        }
    }

    if (curr_sphere == -1) {
        return scene_opts.bg_color;
    }

    Vec3_t intersection_point = vec3_add(
        origin, vec3_scalar_mul(direction, closest_intersection_dist));

    Vec3_t normal = vec3_norm(vec3_sub(
        intersection_point, scene_opts.objects.spheres[curr_sphere].pos));

    Vec3_t reflection_ray = vec3_sub(
        direction, vec3_scalar_mul(normal, 2 * vec3_dot(direction, normal)));

    Vec3_t color = vec3_scalar_mul(
        scene_opts.objects.spheres[curr_sphere].color, AMBIENT_INTENSITY);

    if (curr_sphere == 0) {
        __checkerboard_col(&scene_opts.objects.spheres[curr_sphere],
                           intersection_point, COL_WHITE, COL_BLACK);
    }

    for (unsigned curr_light = 0; curr_light < scene_opts.objects.num_lights; ++curr_light) {
        Vec3_t light_direction = vec3_norm(vec3_sub(
            scene_opts.objects.lights[curr_light].pos, intersection_point));

        bool in_shadow = false;

        for (unsigned i = 0; i < scene_opts.objects.num_spheres && !in_shadow; ++i) {
            in_shadow |= intersect_sphere(scene_opts.objects.spheres[i],
                                          intersection_point, light_direction,
                                          &intersection_dist);
        }

        if (!in_shadow) {
            // Light Direction Normal
            float ldn      = fmaxf(0.0, vec3_dot(light_direction, normal));
            float diffuse  = ldn * DIFFUSE_INTENSITY;
            float specular = powf(ldn, SPECULAR_EXPONENT) * SPECULAR_INTENSITY;

            color = vec3_add(color,
                vec3_scalar_mul(
                    vec3_mul(scene_opts.objects.spheres[curr_sphere].color,
                             scene_opts.objects.lights[curr_light].color), diffuse));

            color = vec3_add(color, Vec3(specular, specular, specular));
        }
    }

    if (ray_depth > 0 && scene_opts.objects.spheres[curr_sphere].is_reflective) {
        Vec3_t reflected_color = trace(scene_opts, intersection_point,
                                       reflection_ray, ray_depth - 1);

        color = vec3_add(
            color, vec3_scalar_mul(reflected_color, REFLECTION_INTENSITY));
    }

    return color;
}

static inline float degree_to_rad(float degrees) {
    return degrees * 3.14159265358979323846 / 180.0;
}

static inline Vec3_t rotate_yaw(Vec3_t v, float yaw) {
    float rad     = degree_to_rad(yaw);
    float cos_yaw = cosf(rad);
    float sin_yaw = sinf(rad);

    return Vec3(v.x * cos_yaw - v.z * sin_yaw, v.y, v.x * sin_yaw + v.z * cos_yaw);
}

static inline Vec3_t rotate_pitch(Vec3_t v, float pitch) {
    float rad       = degree_to_rad(pitch);
    float cos_pitch = cosf(rad);
    float sin_pitch = sinf(rad);

    return Vec3(v.x, v.y * cos_pitch - v.z * sin_pitch, v.y * sin_pitch + v.z * cos_pitch);
}

void render_scene(FILE *fp, SceneOpts_t scene_opts, RenderOpts_t render_opts) {
    assert(scene_opts.objects.lights);
    assert(scene_opts.objects.spheres);
    assert(fp);

    assert(render_opts.yaw <= 360);
    assert(render_opts.pitch >= -90 && render_opts.pitch <= 90);
    assert(render_opts.FOV >= 1 && render_opts.FOV <= 179);
    assert(render_opts.ray_depth <= 10);
    
    assert(scene_opts.width > 0 && scene_opts.height > 0);
    assert(scene_opts.aspect_ratio >= 0.1 && scene_opts.aspect_ratio <= 10);

    float half_height = tanf(degree_to_rad(render_opts.FOV) / 2.0);
    float half_width  = scene_opts.aspect_ratio * half_height;

    for (unsigned y = 0; y < scene_opts.height; ++y) {
        for (unsigned x = 0; x < scene_opts.width; ++x) {
            float screen_x = (2.0 * x / scene_opts.width - 1.0) * half_width;
            float screen_y = -(2.0 * y / scene_opts.height - 1.0) * half_height;

            Vec3_t ray = {
                screen_x, 
                screen_y,
                -1.0  // definir o componente z apontando para a câmera
            };
            
            ray = rotate_yaw(ray, render_opts.yaw);
            ray = rotate_pitch(ray, render_opts.pitch);

            Vec3_t direction = vec3_norm(ray);

            Vec3_t color = trace(scene_opts, render_opts.camera_pos, direction,
                                 render_opts.ray_depth);
            
            unsigned char r = (unsigned char)(fminf(1.0, color.x) * 255);
            unsigned char g = (unsigned char)(fminf(1.0, color.y) * 255);
            unsigned char b = (unsigned char)(fminf(1.0, color.z) * 255);

            fwrite(&r, sizeof(unsigned char), 1, fp);
            fwrite(&g, sizeof(unsigned char), 1, fp);
            fwrite(&b, sizeof(unsigned char), 1, fp);
        }
    }
}

int main(int argc, char const *argv[]) {
    Sphere_t lights[] = {
        Sphere(Vec3(0, 10, 10), COL_YELLOW_LAMP, 0, false),
        Sphere(Vec3(-6, 3, 2),  COL_BLUE,        0, false),
        Sphere(Vec3(-6, 3, 2),  COL_RED,         0, false),
    };

    Sphere_t spheres[] = {
        // chão da cena (com checkerboard)
        Sphere(Vec3(0, -1000, 0), COL_WHITE, 1000, true),
        
        // J
        Sphere(Vec3(-12.5, .5, -10),  COL_RED, .5, true),
        Sphere(Vec3(-11.5, .5, -10),  COL_RED, .5, true),
        Sphere(Vec3(-10.5, .5, -10),  COL_RED, .5, true),
        Sphere(Vec3(-10.5, 1.5, -10), COL_RED, .5, true),
        Sphere(Vec3(-10.5, 2.5, -10), COL_RED, .5, true),
        Sphere(Vec3(-10.5, 3.5, -10), COL_RED, .5, true),
        Sphere(Vec3(-11.5, 3.5, -10), COL_RED, .5, true),
        Sphere(Vec3(-12.5, 3.5, -10), COL_RED, .5, true),
        Sphere(Vec3(-9.5, 3.5, -10),  COL_RED, .5, true),
        Sphere(Vec3(-8.5, 3.5, -10),  COL_RED, .5, true),

        // O
        Sphere(Vec3(-6.5, .5, -10),  COL_RED, .5, true),
        Sphere(Vec3(-6.5, 1.5, -10), COL_RED, .5, true),
        Sphere(Vec3(-6.5, 2.5, -10), COL_RED, .5, true),
        Sphere(Vec3(-6.5, 3.5, -10), COL_RED, .5, true),
        Sphere(Vec3(-5.5, .5, -10),  COL_RED, .5, true),
        Sphere(Vec3(-5.5, 3.5, -10), COL_RED, .5, true),
        Sphere(Vec3(-4.5, .5, -10),  COL_RED, .5, true),
        Sphere(Vec3(-4.5, 3.5, -10), COL_RED, .5, true),
        Sphere(Vec3(-3.5, .5, -10),  COL_RED, .5, true),
        Sphere(Vec3(-3.5, 1.5, -10), COL_RED, .5, true),
        Sphere(Vec3(-3.5, 2.5, -10), COL_RED, .5, true),
        Sphere(Vec3(-3.5, 3.5, -10), COL_RED, .5, true),


        // A
        Sphere(Vec3(-1.5, .5, -10), COL_RED, .5, true),
        Sphere(Vec3(-1, 1.5, -10),  COL_RED, .5, true),
        Sphere(Vec3(-.5, 2.5, -10), COL_RED, .5, true),
        Sphere(Vec3(0, 3.5, -10),   COL_RED, .5, true),
        Sphere(Vec3(.5, 2.5, -10),  COL_RED, .5, true),
        Sphere(Vec3(1, 1.5, -10),   COL_RED, .5, true),
        Sphere(Vec3(1.5, .5, -10),  COL_RED, .5, true),

        // O
        Sphere(Vec3(3.5, .5, -10),  COL_RED, .5, true),
        Sphere(Vec3(3.5, 1.5, -10), COL_RED, .5, true),
        Sphere(Vec3(3.5, 2.5, -10), COL_RED, .5, true),
        Sphere(Vec3(3.5, 3.5, -10), COL_RED, .5, true),
        Sphere(Vec3(4.5, .5, -10),  COL_RED, .5, true),
        Sphere(Vec3(4.5, 3.5, -10), COL_RED, .5, true),
        Sphere(Vec3(5.5, .5, -10),  COL_RED, .5, true),
        Sphere(Vec3(5.5, 3.5, -10), COL_RED, .5, true),
        Sphere(Vec3(6.5, .5, -10),  COL_RED, .5, true),
        Sphere(Vec3(6.5, 1.5, -10), COL_RED, .5, true),
        Sphere(Vec3(6.5, 2.5, -10), COL_RED, .5, true),
        Sphere(Vec3(6.5, 3.5, -10), COL_RED, .5, true),
        
        // objetos miscelãneos
        Sphere(Vec3(0,  3.5, 10),    COL_BLUE,       .5, true),
        Sphere(Vec3(-9, 1, 2),       COL_STELL,       1, true),
        Sphere(Vec3(0,  .5, 5),      COL_BLACK,      .5, false),
        Sphere(Vec3(10, .5, 4),      COL_YELLOW_LAMP, 2, true),
    };

    SceneOpts_t scene_opts = {
        .width        = 3840,
        .height       = 2160,
        .aspect_ratio = (float) scene_opts.width / scene_opts.height,
        .objects      = SceneObjects(lights, spheres),
        .bg_color     = COL_BLACK
    };

    RenderOpts_t render_opts = {
        .camera_pos = Vec3(0, 2, 17),
        .yaw        = -5,
        .pitch      = 1,
        .FOV        = 70, 
        .ray_depth  = 3
    };

    FILE *fp = fopen("render.ppm", "w");

    fprintf(fp, "P6\n%d %d\n255\n", scene_opts.width, scene_opts.height);

    render_scene(fp, scene_opts, render_opts);

    fclose(fp);

    return 0;
}
