#pragma once

#include <cstddef>
#include <vector>

#include <glad/gl.h>
#include <glm/glm.hpp>

namespace Rendering {

    class WeatherRenderer {
    public:
        enum class WeatherType {
            Rain = 0,
            Snow = 1,
        };

        struct Particle {
            glm::vec3 position = glm::vec3(0.0f);
            float speed = 1.0f;
            float random_offset = 0.0f;
        };

        struct Entity {
            WeatherType weather_type = WeatherType::Rain;
            bool enabled = false;
            int particle_count = 300;
            float fall_speed = 1.8f;
            float spawn_radius = 0.22f;
            float earth_radius = 1.0f;
            float rain_length = 0.08f;
            glm::vec3 target_position = glm::vec3(0.0f, 1.5f, 0.0f);
            std::vector<Particle> particles{};
        };

        struct RenderScene {
            glm::mat4 view_projection_matrix = glm::mat4(1.0f);
            std::vector<Entity> entities{};
        };

        WeatherRenderer();
        ~WeatherRenderer();

        WeatherRenderer(const WeatherRenderer&) = delete;
        WeatherRenderer& operator=(const WeatherRenderer&) = delete;

        void render(const RenderScene& scene);

    private:
        struct WeatherVertex {
            glm::vec3 position;
            glm::vec4 colour;
        };

        void ensure_buffer_size(std::size_t vertex_count);
        void upload_vertices(const std::vector<WeatherVertex>& vertices);
        std::vector<WeatherVertex> build_rain_vertices(const Entity& entity) const;
        std::vector<WeatherVertex> build_snow_vertices(const Entity& entity) const;

        GLuint vao;
        GLuint vbo;
        GLuint shader_program;
        std::size_t buffer_vertex_capacity;
    };

}
