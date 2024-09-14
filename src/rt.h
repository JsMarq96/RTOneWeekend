#pragma once

#include <glm/glm.hpp>

struct sRay {
    glm::dvec3 origin;
    glm::dvec3 dir;

    inline glm::dvec3 at(const double t) const {
        return origin + t * dir;
    }
};

glm::dvec3 get_ray_color(const sRay& r) {
    glm::dvec3 resulting_color;

    // Sky color
    const glm::vec3 dir = glm::normalize(r.dir); 
    double a = 0.5*(dir.y + 1.0);
    resulting_color = (1.0-a)*glm::dvec3{1.0, 1.0, 1.0} + a*glm::dvec3{0.5, 0.7, 1.0};

    return resulting_color;
}