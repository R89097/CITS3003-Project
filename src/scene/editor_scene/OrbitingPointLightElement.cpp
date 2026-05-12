

#include "OrbitingPointLightElement.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include <GLFW/glfw3.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/component_wise.hpp>

#include "rendering/imgui/ImGuiManager.h"
#include "scene/SceneContext.h"

namespace {
    constexpr float PI = 3.14159265358979323846f;
}

std::unique_ptr<EditorScene::OrbitingPointLightElement> EditorScene::OrbitingPointLightElement::new_default(const SceneContext& scene_context, EditorScene::ElementRef parent) {
    auto light_element = std::make_unique<OrbitingPointLightElement>(
        parent,
        "New Orbiting Point Light",
        glm::vec3{0.0f, 1.0f, 0.0f},
        2.0f,
        0.0f,
        5.0f,
        PointLight::create(
            glm::vec3{}, // Set via update_instance_data()
            glm::vec4{1.0f}
        ),
        EmissiveEntityRenderer::Entity::create(
            scene_context.model_loader.load_from_file<EmissiveEntityRenderer::VertexData>("sphere.obj"),
            EmissiveEntityRenderer::InstanceData{
                glm::mat4{}, // Set via update_instance_data()
                EmissiveEntityRenderer::EmissiveEntityMaterial{
                    glm::vec4{1.0f}
                }
            },
            EmissiveEntityRenderer::RenderData{
                scene_context.texture_loader.default_white_texture()
            }
        )
    );

    light_element->update_instance_data();
    return light_element;
}

std::unique_ptr<EditorScene::OrbitingPointLightElement> EditorScene::OrbitingPointLightElement::from_json(const SceneContext& scene_context, EditorScene::ElementRef parent, const json& j) {
    auto light_element = new_default(scene_context, parent);

    light_element->orbit_center = j["orbit_center"];
    light_element->orbit_radius = j["orbit_radius"];
    light_element->orbit_tilt_angle = j["orbit_tilt_angle"];
    light_element->orbit_duration = j["orbit_duration"];
    light_element->light->colour = j["colour"];
    light_element->visible = j["visible"];
    light_element->visual_scale = j["visual_scale"];

    light_element->update_instance_data();
    return light_element;
}

json EditorScene::OrbitingPointLightElement::into_json() const {
    return {
        {"orbit_center",     orbit_center},
        {"orbit_radius",     orbit_radius},
        {"orbit_tilt_angle", orbit_tilt_angle},
        {"orbit_duration",   orbit_duration},
        {"colour",           light->colour},
        {"visible",          visible},
        {"visual_scale",     visual_scale},
    };
}

glm::vec3 EditorScene::OrbitingPointLightElement::current_position(float time_seconds) const {
    float safe_duration = std::max(orbit_duration, 0.001f);
    float angle = (time_seconds / safe_duration) * 2.0f * PI;

    float tilt = glm::radians(orbit_tilt_angle);

    glm::vec3 orbit_offset{
        std::cos(angle) * orbit_radius,
        std::sin(angle) * orbit_radius * std::sin(tilt),
        std::sin(angle) * orbit_radius * std::cos(tilt)
    };

    return orbit_center + orbit_offset;
}

void EditorScene::OrbitingPointLightElement::add_imgui_edit_section(MasterRenderScene& render_scene, const SceneContext& scene_context) {
    ImGui::Text("Orbiting Point Light");
    SceneElement::add_imgui_edit_section(render_scene, scene_context);

    bool transformUpdated = false;

    ImGui::Text("Orbit Settings");
    transformUpdated |= ImGui::DragFloat3("Orbit Center", &orbit_center[0], 0.01f);
    ImGui::DragDisableCursor(scene_context.window);

    transformUpdated |= ImGui::DragFloat("Orbit Radius", &orbit_radius, 0.01f, 0.0f, FLT_MAX);
    ImGui::DragDisableCursor(scene_context.window);

    transformUpdated |= ImGui::DragFloat("Orbit Tilt Angle", &orbit_tilt_angle, 0.1f, -90.0f, 90.0f);
    ImGui::DragDisableCursor(scene_context.window);

    transformUpdated |= ImGui::DragFloat("Orbit Duration", &orbit_duration, 0.01f, 0.001f, FLT_MAX);
    ImGui::DragDisableCursor(scene_context.window);
    ImGui::Spacing();

    ImGui::Text("Light Properties");
    transformUpdated |= ImGui::ColorEdit3("Colour", &light->colour[0]);
    ImGui::Spacing();
    transformUpdated |= ImGui::DragFloat("Intensity", &light->colour.a, 0.01f, 0.0f, FLT_MAX);
    ImGui::DragDisableCursor(scene_context.window);

    ImGui::Spacing();
    ImGui::Text("Visuals");

    transformUpdated |= ImGui::Checkbox("Show Visuals", &visible);
    transformUpdated |= ImGui::DragFloat("Visual Scale", &visual_scale, 0.01f, 0.0f, FLT_MAX);
    ImGui::DragDisableCursor(scene_context.window);

    if (transformUpdated) {
        update_instance_data();
    }
}

void EditorScene::OrbitingPointLightElement::update_instance_data() {
    glm::vec3 position = current_position(static_cast<float>(glfwGetTime()));

    float safe_duration = std::max(orbit_duration, 0.001f);
    float cycle = std::fmod(static_cast<float>(glfwGetTime()) / orbit_duration, 1.0f);

    glm::vec4 sunrise_colour = glm::vec4(1.0f, 0.55f, 0.25f, 0.8f);
    glm::vec4 day_colour     = glm::vec4(1.0f, 0.95f, 0.8f, 1.0f);
    glm::vec4 sunset_colour  = glm::vec4(1.0f, 0.4f, 0.2f, 0.75f);
    glm::vec4 night_colour   = glm::vec4(0.2f, 0.3f, 0.65f, 0.45f);

    glm::vec4 dynamic_colour;

    if (cycle < 0.25f) {
        float t = cycle / 0.25f;
        dynamic_colour = glm::mix(sunrise_colour, day_colour, t);
    }
    else if (cycle < 0.5f) {
        float t = (cycle - 0.25f) / 0.25f;
        dynamic_colour = glm::mix(day_colour, sunset_colour, t);
    }
    else if (cycle < 0.75f) {
        float t = (cycle - 0.5f) / 0.25f;
        dynamic_colour = glm::mix(sunset_colour, night_colour, t);
    }
    else {
        float t = (cycle - 0.75f) / 0.25f;
        dynamic_colour = glm::mix(night_colour, sunrise_colour, t);
    }

    light->colour = dynamic_colour;
    
    transform = glm::translate(position);

    if (!EditorScene::is_null(parent)) {
        // Post multiply by transform so that local transformations are applied first
        transform = (*parent)->transform * transform;
    }

    light->position = glm::vec3(transform[3]); // Extract translation from matrix
    if (visible) {
        light_sphere->instance_data.model_matrix = transform * glm::scale(glm::vec3{0.1f * visual_scale});
    } else {
        // Throw off to infinity as a hacky way to make model invisible
        light_sphere->instance_data.model_matrix = glm::scale(glm::vec3{std::numeric_limits<float>::infinity()}) * glm::translate(glm::vec3{std::numeric_limits<float>::infinity()});
    }

    glm::vec3 colour = glm::vec3(light->colour);
    float max_colour = glm::compMax(colour);
    glm::vec3 normalised_colour = max_colour > 0.0f ? colour / max_colour : glm::vec3{1.0f};
    light_sphere->instance_data.material.emission_tint = glm::vec4(normalised_colour, light_sphere->instance_data.material.emission_tint.a);
}

const char* EditorScene::OrbitingPointLightElement::element_type_name() const {
    return ELEMENT_TYPE_NAME;
}