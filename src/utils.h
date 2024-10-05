#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

inline double random_double() {
    return glm::linearRand(0.0, 1.0);
}