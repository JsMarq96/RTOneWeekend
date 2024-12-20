#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <cfloat>

#include "camera.h"
#include "utils.h"

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

namespace intersections {
    // TODO: comment
    double ray_sphere(const glm::dvec3 &sphere_center, const double sphere_radius, const sRay &ray) {
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

class Hitable {
public:
    virtual ~Hitable() = default;

    virtual bool hit(const sRay& r, const sInterval &t_int, sHitRecord& result) const = 0;
};

class Sphere : public Hitable {
public:
    Sphere(const glm::dvec3 &center, const double radius) : center(center), radius(glm::max(0.0, radius)) {}


    bool hit(const sRay& ray, const sInterval &t_int, sHitRecord& result) const override {
        const glm::dvec3 rorigin_scenter = center - ray.origin;
        const double a = glm::length2(ray.dir);
        const double h = glm::dot(ray.dir, rorigin_scenter);
        const double c = glm::length2(rorigin_scenter) - radius * radius;

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
        result.set_normal(ray, (result.p - center) / radius);

        return true;
    }
private:
    glm::dvec3 center;
    double radius;
};


class HittableList : public Hitable {
    public:
    uint32_t hitable_count = 0u;
    const Hitable* hitable_list[200];

    void add(const Hitable* obj) {
        hitable_list[hitable_count++] = obj;
    }

    bool hit(const sRay& ray, const sInterval &t_int, sHitRecord& result) const override {
        sHitRecord temp_rec;

        bool hit_anything = false;
        double closest_so_far = t_int.max;

        for(uint32_t i = 0u; i < hitable_count; i++) {
            if (hitable_list[i]->hit(ray, {.min = t_int.min, .max = closest_so_far}, temp_rec)) {
                hit_anything = true;
                closest_so_far = temp_rec.t;
                result = temp_rec;
            }
        }

        return hit_anything;
    }
};


glm::dvec3 get_ray_color(const sRay& r, const HittableList& world) {
    glm::dvec3 resulting_color;

    // Sky color
    const glm::vec3 dir = glm::normalize(r.dir); 
    double a = 0.5*(dir.y + 1.0);
    resulting_color = (1.0-a)*glm::dvec3{1.0, 1.0, 1.0} + a*glm::dvec3{0.5, 0.7, 1.0};

    sHitRecord record;
    if (world.hit(r, {.min = 0.0, .max = DBL_MAX}, record)) {
        resulting_color = 0.5 * (record.normal + glm::dvec3(1.0)); 
    }

    return resulting_color;
}

glm::dvec3 render_pixel(const uint32_t i, const uint32_t j, const sCamera& camera, const HittableList& world, const uint8_t sample_count = 10u) {
    glm::dvec3 ray_color = {0.0, 0.0, 0.0};

    for(uint8_t s = 0u; s < sample_count; s++) {
        // Supersampling pixel jitter
        const glm::dvec2 square_samples = (sample_count > 1) ? glm::linearRand(glm::dvec2{0.0, 0.0}, glm::dvec2{1.0, 1.0}) : glm::dvec2{0.0, 0.0};
        
        const glm::dvec3 pixel_center = camera.pixel00_pos + ((glm::dvec3(i) + square_samples.x) * camera.pixel_delta_u) + ((glm::dvec3(j) + square_samples.y) * camera.pixel_delta_v);
        const glm::dvec3 ray_direction = pixel_center - camera.center;

        ray_color += get_ray_color({.origin = camera.center, .dir = ray_direction}, world);
    }

    return ray_color / ((double) sample_count);
}