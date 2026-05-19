#include "scene/editor_scene/WeatherElement.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <utility>

#include <glm/gtx/norm.hpp>
#include <imgui.h>

#include "scene/SceneContext.h"

namespace {
    float random_float(float min_value, float max_value) {
        float t = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        return min_value + t * (max_value - min_value);
    }

    glm::vec3 sphere_point(float lat_deg, float lon_deg, float radius) {
        float lat = glm::radians(lat_deg);
        float lon = glm::radians(lon_deg);

        return glm::vec3(
            radius * std::cos(lat) * std::cos(lon),
            radius * std::sin(lat),
            radius * std::cos(lat) * std::sin(lon)
        );
    }

    const char* weather_type_names[] = {
        "Rain",
        "Snow",
    };

    const char* cloud_names[] = {
        "Cloud 1",
        "Cloud 2",
        "Cloud 3",
    };

    glm::vec3 cloud_position_from_name(const std::string& name) {
        // These positions match the three visible cloud/patch locations generated around the planet.
        // The particles use these centres as spawn areas, then fall toward the planet centre.
        if (name == "Cloud 1") {
            return sphere_point(25.0f, 40.0f, 1.62f);
        }
        if (name == "Cloud 2") {
            return sphere_point(-10.0f, 120.0f, 1.62f);
        }
        if (name == "Cloud 3") {
            return sphere_point(35.0f, 210.0f, 1.62f);
        }

        return sphere_point(25.0f, 40.0f, 1.62f);
    }
}

std::unique_ptr<EditorScene::WeatherElement> EditorScene::WeatherElement::new_default(const SceneContext& scene_context, EditorScene::ElementRef parent) {
    (void) scene_context;

    return std::make_unique<WeatherElement>(
        parent,
        "New Cloud Weather",
        "Cloud 1",
        WeatherType::Rain,
        true,
        250,
        1.8f,
        0.22f,
        1.0f,
        0.08f
    );
}

EditorScene::WeatherElement::WeatherElement(
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
) :
    SceneElement(parent, std::move(name)),
    target_cloud_name(std::move(target_cloud_name)),
    weather_type(weather_type),
    enabled(enabled),
    particle_count(particle_count),
    fall_speed(fall_speed),
    spawn_radius(spawn_radius),
    earth_radius(earth_radius),
    rain_length(rain_length),
    time(0.0f)
{
    resize_particles();
}

void EditorScene::WeatherElement::update(float delta_time) {
    if (!enabled) {
        return;
    }

    time += delta_time;
    resize_particles();

    for (Particle& particle : particles) {
        glm::vec3 fall_direction = glm::normalize(-particle.position);

        if (weather_type == WeatherType::Snow) {
            glm::vec3 tangent = glm::cross(fall_direction, glm::vec3(0.0f, 1.0f, 0.0f));
            if (glm::length2(tangent) < 0.0001f) {
                tangent = glm::cross(fall_direction, glm::vec3(1.0f, 0.0f, 0.0f));
            }
            tangent = glm::normalize(tangent);

            float sway = std::sin(time * 2.0f + particle.random_offset) * 0.12f;
            particle.position += tangent * sway * delta_time;
        }

        particle.position += fall_direction * particle.speed * delta_time;

        if (glm::length(particle.position) <= earth_radius) {
            reset_particle(particle);
        }
    }
}

void EditorScene::WeatherElement::draw_imgui(const SceneContext& scene_context) {
    (void) scene_context;

    ImGui::Checkbox("Enable Weather", &enabled);

    int weather_type_index = static_cast<int>(weather_type);
    if (ImGui::Combo("Weather Type", &weather_type_index, weather_type_names, 2)) {
        weather_type = static_cast<WeatherType>(weather_type_index);
        for (Particle& particle : particles) {
            reset_particle(particle);
        }
    }

    int selected_cloud_index = 0;
    if (target_cloud_name == "Cloud 2") {
        selected_cloud_index = 1;
    } else if (target_cloud_name == "Cloud 3") {
        selected_cloud_index = 2;
    }

    if (ImGui::Combo("Target Cloud", &selected_cloud_index, cloud_names, 3)) {
        target_cloud_name = cloud_names[selected_cloud_index];
        for (Particle& particle : particles) {
            reset_particle(particle);
        }
    }

    if (ImGui::SliderInt("Particle Count", &particle_count, 20, 1500)) {
        particle_count = std::max(1, particle_count);
        resize_particles();
    }

    ImGui::SliderFloat("Fall Speed", &fall_speed, 0.1f, 8.0f);
    ImGui::SliderFloat("Spawn Radius", &spawn_radius, 0.02f, 1.0f);
    ImGui::SliderFloat("Earth Radius", &earth_radius, 0.2f, 3.0f);
    ImGui::SliderFloat("Rain Length", &rain_length, 0.01f, 0.3f);
}

const std::vector<EditorScene::WeatherElement::Particle>& EditorScene::WeatherElement::get_particles() const {
    return particles;
}

EditorScene::WeatherElement::WeatherType EditorScene::WeatherElement::get_weather_type() const {
    return weather_type;
}

bool EditorScene::WeatherElement::is_enabled() const {
    return enabled;
}

float EditorScene::WeatherElement::get_rain_length() const {
    return rain_length;
}

void EditorScene::WeatherElement::resize_particles() {
    int safe_particle_count = std::max(1, particle_count);
    particle_count = safe_particle_count;

    std::size_t old_size = particles.size();
    particles.resize(static_cast<std::size_t>(safe_particle_count));

    for (std::size_t i = old_size; i < particles.size(); ++i) {
        reset_particle(particles[i]);
    }
}

void EditorScene::WeatherElement::reset_particle(Particle& particle) {
    glm::vec3 cloud_position = cloud_position_from_name(target_cloud_name);
    glm::vec3 outward = glm::normalize(cloud_position);
    glm::vec3 fall_direction = -outward;

    glm::vec3 tangent_a = glm::cross(outward, glm::vec3(0.0f, 1.0f, 0.0f));
    if (glm::length2(tangent_a) < 0.0001f) {
        tangent_a = glm::cross(outward, glm::vec3(1.0f, 0.0f, 0.0f));
    }
    tangent_a = glm::normalize(tangent_a);
    glm::vec3 tangent_b = glm::normalize(glm::cross(outward, tangent_a));

    float offset_a = random_float(-spawn_radius, spawn_radius);
    float offset_b = random_float(-spawn_radius, spawn_radius);
    float below_cloud = random_float(0.0f, spawn_radius * 0.4f);

    particle.position = cloud_position + tangent_a * offset_a + tangent_b * offset_b + fall_direction * below_cloud;

    float speed_multiplier = weather_type == WeatherType::Rain ? random_float(0.8f, 1.3f) : random_float(0.4f, 0.8f);
    particle.speed = fall_speed * speed_multiplier;
    particle.random_offset = random_float(0.0f, 100.0f);
}

glm::vec3 EditorScene::WeatherElement::get_target_cloud_position(const SceneContext& scene_context) const {
    (void) scene_context;
    return cloud_position_from_name(target_cloud_name);
}
