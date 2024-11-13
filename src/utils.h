#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

#ifdef _WIN32
#include <chrono>
#else
#include <sys/time.h>
#endif

inline double random_double() {
    return glm::linearRand(0.0, 1.0);
}
inline uint64_t get_timestamp_microsecs() {
#ifdef _WIN32
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
#else
    struct timeval stamp;
    gettimeofday(&stamp, NULL);

    return stamp.tv_sec * 1000000u + stamp.tv_usec;
#endif
}