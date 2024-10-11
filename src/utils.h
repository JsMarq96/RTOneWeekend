#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

#include <sys/time.h>

inline double random_double() {
    return glm::linearRand(0.0, 1.0);
}
inline uint64_t get_timestamp_microsecs() {
    struct timeval stamp;
    gettimeofday(&stamp, NULL);

    return stamp.tv_sec * 1000000u + stamp.tv_usec;
}