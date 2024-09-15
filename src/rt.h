#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include <cfloat>

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
    }
};

namespace intersections {
    enum eHitTypes : uint8_t {
        SPHERE,
        HITTYPES_COUNT
    };

    // TODO: comment
    double ray_sphere(const sRay &ray, const glm::dvec3 &sphere_center, const double sphere_radius) {
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

    double ray_sphere_closest(const sRay &ray, const glm::dvec3 &sphere_center, const double sphere_radius, const sInterval &t_int, sHitRecord &result) {
        const glm::dvec3 rorigin_scenter = sphere_center - ray.origin;
        const double a = glm::length2(ray.dir);
        const double h = glm::dot(ray.dir, rorigin_scenter);
        const double c = glm::length2(rorigin_scenter) - sphere_radius * sphere_radius;

        const double discriminant = h * h - a * c;

        if (discriminant < 0) {
            return -1.0;
        }

        const double sqrt_dis = glm::sqrt(discriminant);
        double root = (h - sqrt_dis) / a;

        // Find the closest (and valid) root
        if (!t_int.surrounds(root)) {
            root = (h + sqrt_dis) / a;

            if (!t_int.surrounds(root)) {
                return -1.0;
            }
        }

        result.t = root;
        result.p = ray.at(root);
        result.set_normal(ray, (result.p - sphere_center) / sphere_radius);

        return root;
    };
};

struct sHittableWorld {
    uint32_t    elem_count  = 0u;

    intersections::eHitTypes    elem_types  [HITTABLE_WORLD_SIZE] = {};
    glm::dvec3                  position    [HITTABLE_WORLD_SIZE] = {};
    double                      radius      [HITTABLE_WORLD_SIZE] = {};

    inline uint32_t add_sphere(const glm::dvec3 &center, const double r) {
        elem_types[elem_count] = intersections::SPHERE;
        position[elem_count] = center;
        radius[elem_count] = r;

        return elem_count++;
    }

    bool ray_hit(const sRay &ray, const sInterval &t_int, sHitRecord &result, uint32_t *idx) const {\
        sHitRecord temp_rec = {};

        bool hit_anything = false;
        double closest_so_far = t_int.max;

        for(uint32_t i = 0u; i < elem_count; i++) {
            double root = 0.0;

            // Perform intersection tests
            switch(elem_types[i]) {
                case intersections::SPHERE:
                    root = intersections::ray_sphere_closest(ray, position[i], radius[i], {.min = t_int.min, .max = closest_so_far}, temp_rec);
                    break;
                default:
                break;
            }

            if (root > 0.0) {
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
};