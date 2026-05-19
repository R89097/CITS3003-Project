

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "scene/editor_scene/SceneElement.h"

class SceneContext;

namespace EditorScene {

    class WeatherElement : public SceneElement {
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

        static std::unique_ptr<WeatherElement> new_default(const SceneContext& scene_context, ElementRef parent);

        WeatherElement(
            ElementRef parent,
            std::string name,
            std::string target_cloud_name,
            WeatherType weather_type,
            bool enabled,
            int particle_count,
            float fall_speed,
            float spawn_radius,
            float earth_radius,
            float rain_length
        );

        void update(float delta_time);
        void draw_imgui(const SceneContext& scene_context) override;

        const std::vector<Particle>& get_particles() const;
        WeatherType get_weather_type() const;
        bool is_enabled() const;
        float get_rain_length() const;

    private:
        void resize_particles();
        void reset_particle(Particle& particle);
        glm::vec3 get_target_cloud_position(const SceneContext& scene_context) const;

        std::string target_cloud_name;
        WeatherType weather_type;
        bool enabled;
        int particle_count;
        float fall_speed;
        float spawn_radius;
        float earth_radius;
        float rain_length;
        float time;
        std::vector<Particle> particles;
    };

}