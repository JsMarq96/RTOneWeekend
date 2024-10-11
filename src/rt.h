#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include <cfloat>

#include "camera.h"
#include "utils.h"

#define HITTABLE_WORLD_SIZE 200u

struct sRay {
    glm::dvec3 origin;
    glm::dvec3 dir;

    inline glm::dvec3 at(const double t) const {
        return origin + t * dir;
    }
};

struct sInterval {
    double min = DBL_MAX;
    double max = -DBL_MAX;

    inline double clamp(const double v) const {
        if (v < min) return min;
        if (v > max) return max;
        return v;
    }

    inline bool contains(const double v) const {
        return min <= v && v <= max;
    }

    inline bool surrounds(const double v) const {
        return min < v && v < max;
    }

    inline double size() const {
        return max - min;
    }
};

struct sHitRecord {
    glm::dvec3 p;
    glm::dvec3 normal;
    double t;
    bool front_face;

    void set_normal(const sRay &ray, const glm::dvec3 &outward_normal) {
        front_face = glm::dot(ray.dir, outward_normal) < 0.0;
        normal = (front_face) ? outward_normal : -outward_normal;
        normal = glm::normalize(normal);
    }
};

namespace intersections {
    enum eHitTypes : uint8_t {
        SPHERE,
        HITTYPES_COUNT
    };

    // TODO: comment
    double ray_sphere(  const sRay &ray, 
                        const glm::dvec3 &sphere_center, 
                        const double sphere_radius) {
        const glm::dvec3 rorigin_scenter = sphere_center - ray.origin;
        const double a = glm::length2(ray.dir);
        const double h = glm::dot(ray.dir, rorigin_scenter);
        const double c = glm::length2(rorigin_scenter) - sphere_radius * sphere_radius;

        const double discriminant = h * h - a * c;

        if (discriminant < 0) {
            return -1.0;
        }

        return (h - glm::sqrt(discriminant)) / a;
    };

    bool ray_sphere_closest(  const sRay &ray, 
                                const glm::dvec3 &sphere_center, 
                                const double sphere_radius, 
                                const sInterval &t_int, 
                                sHitRecord &result) {
        const glm::dvec3 rorigin_scenter = sphere_center - ray.origin;
        const double a = glm::length2(ray.dir);
        const double h = glm::dot(ray.dir, rorigin_scenter);
        const double c = glm::length2(rorigin_scenter) - sphere_radius * sphere_radius;

        const double discriminant = h * h - a * c;

        if (discriminant < 0) {
            return false;
        }

        const double sqrt_dis = glm::sqrt(discriminant);
        double root = (h - sqrt_dis) / a;

        // Find the closest (and valid) root
        if (!t_int.surrounds(root)) {
            root = (h + sqrt_dis) / a;

            if (!t_int.surrounds(root)) {
                return false;
            }
        }

        result.t = root;
        result.p = ray.at(root);
        result.set_normal(ray, (result.p - sphere_center) / sphere_radius);

        return true;
    };
};

struct sHittableWorld {
    uint32_t    elem_count  = 0u;

    intersections::eHitTypes    elem_types  [HITTABLE_WORLD_SIZE] = {};
    glm::dvec3                  position    [HITTABLE_WORLD_SIZE] = {};
    double                      radius      [HITTABLE_WORLD_SIZE] = {};

    // Supersampling buffers
    glm::dvec3 sample_color_buffer[255] = {};
    sRay sample_ray_buffer[255] = {};

    inline uint32_t add_sphere( const glm::dvec3 &center, 
                                const double r) {
        elem_types[elem_count] = intersections::SPHERE;
        position[elem_count] = center;
        radius[elem_count] = r;

        return elem_count++;
    }

    bool ray_hit(   const sRay &ray, 
                    const sInterval &t_int, 
                    sHitRecord &result, 
                    uint32_t *idx   ) const {
        sHitRecord temp_rec = {};

        bool hit_anything = false;
        double closest_so_far = t_int.max;

        for(uint32_t i = 0u; i < elem_count; i++) {
            bool has_hit = false;

            // Perform intersection tests
            switch(elem_types[i]) {
                case intersections::SPHERE:
                    has_hit = intersections::ray_sphere_closest(   ray, 
                                                                position[i], 
                                                                radius[i], 
                                                                {.min = t_int.min, .max = closest_so_far}, 
                                                                temp_rec    );
                    break;
                default:
                break;
            }

            if (has_hit) {
                hit_anything = true;
                closest_so_far = temp_rec.t;
                result = temp_rec;
                *idx = i;
            }
        }

        return hit_anything;
    }

    glm::dvec3 get_ray_color(const sRay &ray) const {
        glm::dvec3 resulting_color;

        // Sky color
        const glm::vec3 dir = glm::normalize(ray.dir); 
        double a = 0.5*(dir.y + 1.0);
        resulting_color = (1.0 - a) * glm::dvec3{1.0, 1.0, 1.0} + a * glm::dvec3{0.5, 0.7, 1.0};

        sHitRecord record;
        uint32_t idx;
        if (ray_hit(ray,{.min = 0.0, .max = DBL_MAX}, record, &idx)) {
            resulting_color = 0.5 * (record.normal + glm::dvec3(1.0)); 
        }

        return resulting_color;
    }

    glm::dvec3 get_pixel_color( const uint32_t i, 
                                const uint32_t j, 
                                const sCamera &camera, 
                                const uint8_t sample_count = 1u) {
        glm::dvec3 result_color = {0.0, 0.0, 0.0};

        assert(sample_count >= 1u && "The sample count needs to be at least 1");

        // Generate all the sample position
        for(uint8_t s = 0u; s < sample_count; s++) {
            const glm::dvec2 square_samples = (sample_count > 1) ? glm::linearRand(glm::dvec2{0.0, 0.0}, glm::dvec2{0.5, 0.5}) : glm::dvec2{0.0, 0.0};
            const glm::dvec3 r_origin = camera.pixel00_pos + ((glm::dvec3(i) + square_samples.x) * camera.pixel_delta_u) + ((glm::dvec3(j) + square_samples.y) * camera.pixel_delta_v);

            sample_ray_buffer[s] = {.origin = camera.center, .dir = glm::normalize(r_origin - camera.center)};
        }

        // Launch rays
        for(uint8_t s = 0u; s < sample_count; s++) {
            sample_color_buffer[s] = get_ray_color(sample_ray_buffer[s]);
        }

        // Compute the supersampling
        for(uint8_t s = 0u; s < sample_count; s++) {
            result_color += sample_color_buffer[s];
        }

        return result_color / ((double) sample_count);
    }
};