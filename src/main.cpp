#include <iostream>
#include <spdlog/spdlog.h>
#include <cstdint>
#include <cstdint>

#include "camera.h"

#include "rt.h"

#define DEFAULT_OUTPUT_IMAGE "rt_one_week_res.ppm"

void write_ppm(const double* raw_data, const uint32_t width, const uint32_t height, const char* file_name = nullptr);

int main() {
    sCamera camera;
    camera.intialize(image_data);

    glm::dvec3 *data = new glm::dvec3[image_data.width * image_data.height];

    HittableList world;

    world.add(new Sphere({0.0, 0.0, -1.0}, 0.5));
    world.add(new Sphere({0.0, -100.5, -1.0}, 100.0));

    spdlog::info("Computing render, starting timer");

    const uint64_t start_time = get_timestamp_microsecs();

    for (int j = 0; j < image_data.height; j++) {
        for (int i = 0; i < image_data.width; i++) {
            const int32_t idx = (i + image_data.width *j);
            data[idx] = render_pixel(i, j, camera, world, 50u);
        }
    }
    const uint64_t end_time = get_timestamp_microsecs();

    spdlog::info("Finished render: resulting time {} microseconds", end_time - start_time);
    spdlog::info("Saving image");

    write_ppm((double*) data, image_data.width, image_data.height);
    
    delete[] data;
    return 0u;
}

// HELPER FUNCTIONS ======================

inline void write_ppm(  const double* raw_data, 
                        const uint32_t width,
                        const uint32_t height, 
                        const char* file_name) {
    FILE* file = fopen((file_name) ? file_name : DEFAULT_OUTPUT_IMAGE, "w");

    // Write header with format
    fprintf(file, "P3\n%d %d\n255\n", width, height);

    const sInterval intensity_interval = {.min = 0.0, .max = 0.999}; 
    // Write the clamped img data
    for(uint32_t j = 0u; j < height; j++) {
        for(uint32_t i = 0u; i < width; i++) {
            const uint32_t idx = (i + width *j) * 3u;
            fprintf(file, 
                    "%d %d %d\n", 
                    uint32_t(255.999 * intensity_interval.clamp(raw_data[idx])), 
                    uint32_t(255.999 * intensity_interval.clamp(raw_data[idx+1u])), 
                    uint32_t(255.999 * intensity_interval.clamp(raw_data[idx+2u])));
        }
    }

    fclose(file);
}