#pragma once

#include <glm/glm.hpp>

static struct sResultImageData {
    const double aspect_ratio = (16.0 / 9.0);
    const uint32_t width = 400;
    const uint32_t height = int(width / aspect_ratio);
} image_data;


struct sCamera {
    glm::dvec3 center = {0.0, 0.0, 0.0};
    
    double focal_length = 1.0;
    double viewport_height = 2.0;
    double viewport_width = 0.0;

    // Vectors for traversing the viewport
    glm::dvec3 viewport_u;
    glm::dvec3 viewport_v;

    glm::dvec3 viewport_upper_left;

    glm::dvec3 pixel_delta_u;
    glm::dvec3 pixel_delta_v;

    glm::dvec3 pixel00_pos;

    void intialize(const sResultImageData &result_format) {
        focal_length = 1.0;
        viewport_height = 2.0;

        viewport_width = viewport_height * (double(result_format.width) / result_format.height);

        viewport_u = {viewport_width, 0.0, 0.0};
        viewport_v = {0.0, -viewport_height, 0.0};

        viewport_upper_left = center - glm::dvec3(0.0, 0.0, focal_length) - (viewport_u/glm::dvec3(2)) - (viewport_v/glm::dvec3(2));

        pixel_delta_u = viewport_u / glm::dvec3(result_format.width);
        pixel_delta_v = viewport_v / glm::dvec3(result_format.height);

        pixel00_pos = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);
    };
};