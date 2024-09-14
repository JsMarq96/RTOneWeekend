#include <iostream>
#include <spdlog/spdlog.h>
#include <cstdint>
#include <cstdint>

#include "rt.h"

#define DEFAULT_OUTPUT_IMAGE "rt_one_week_res.ppm"
struct sResultImageData {
    const double aspect_ratio = (16.0 / 9.0);
    const uint32_t width = 400;
    const uint32_t height = int(width / aspect_ratio);
} image_data;

struct sCameraData {
    glm::dvec3 center = {0.0, 0.0, 0.0};
    
    double focal_length = 1.0;
    double viewport_height = 2.0;
    double viewport_width = viewport_height * (double(image_data.width) / image_data.height);

    // Vectors for traversing the viewport
    glm::dvec3 viewport_u = {viewport_width, 0.0, 0.0};
    glm::dvec3 viewport_v = {0.0, -viewport_height, 0.0};

    glm::dvec3 viewport_upper_left = center - glm::dvec3(0.0, 0.0, focal_length) - (viewport_u/glm::dvec3(2)) - (viewport_v/glm::dvec3(2));

    glm::dvec3 pixel_delta_u = viewport_u / glm::dvec3(image_data.width);
    glm::dvec3 pixel_delta_v = viewport_v / glm::dvec3(image_data.height);

    glm::dvec3 pixel00_pos = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

} camera;


const char *ppm_sample = "P3\n 4 4\n 15\n0  0  0    0  0  0    0  0  0   15  0 15\n0  0  0    0 15  7    0  0  0    0  0  0\n0  0  0    0  0  0    0 15  7    0  0  0\n15  0 15    0  0  0    0  0  0    0  0  0";

inline void write_ppm(  const double* raw_data, 
                        const uint32_t width,
                        const uint32_t height, 
                        const char* file_name = nullptr) {
    FILE* file = fopen((file_name) ? file_name : DEFAULT_OUTPUT_IMAGE, "w");

    // Write header
    fprintf(file, "P3\n%d %d\n255\n", width, height);

    // Load the img data
    for(uint32_t j = 0u; j < height; j++) {
        for(uint32_t i = 0u; i < width; i++) {
            const uint32_t idx = (i + width *j) * 3u;
            fprintf(file, "%d %d %d\n", uint32_t(255.999 * raw_data[idx]), uint32_t(255.999 * raw_data[idx+1u]), uint32_t(255.999 * raw_data[idx+2u]));
        }
    }

    fclose(file);
}

int main() {
    double *data = new double[image_data.width * image_data.height * 3];

    spdlog::info("Computing render");

    for (int j = 0; j < image_data.height; j++) {
        for (int i = 0; i < image_data.width; i++) {
            glm::dvec3 pixel_center = camera.pixel00_pos + (camera.pixel_delta_u * glm::dvec3(i)) + (glm::dvec3(j) * camera.pixel_delta_v);
            glm::dvec3 ray_direction = pixel_center - camera.center;

            glm::dvec3 pixel_color = get_ray_color({.origin = pixel_center, .dir = ray_direction});

            const int32_t idx = (i + image_data.width *j) * 3;

            data[idx] = pixel_color.x;
            data[idx+1] = pixel_color.y;
            data[idx+2] = pixel_color.z;
        }
    }

    spdlog::info("Saving image");

    write_ppm(data, image_data.width, image_data.height);
    
    delete[] data;
    return 0u;
}