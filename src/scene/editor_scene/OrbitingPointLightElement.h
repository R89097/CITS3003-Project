

#ifndef ORBITING_POINT_LIGHT_ELEMENT_H
#define ORBITING_POINT_LIGHT_ELEMENT_H

#include "SceneElement.h"
#include "scene/SceneContext.h"

namespace EditorScene {
    class OrbitingPointLightElement : public SceneElement {
    public:
        /// NOTE: Must be unique per element type, as it is used to select generators,
        ///       so if you are creating a new element type make sure to change this to a new unique name.
        static constexpr const char* ELEMENT_TYPE_NAME = "Orbiting Point Light";

        // Orbit settings
        glm::vec3 orbit_center;
        float orbit_radius = 2.0f;
        float orbit_tilt_angle = 0.0f;
        float orbit_duration = 5.0f;

        bool visible = true;
        float visual_scale = 1.0f;

        // This is still stored/rendered as a normal PointLight.
        // The only difference is that its position will be updated every frame.
        std::shared_ptr<PointLight> light;
        std::shared_ptr<EmissiveEntityRenderer::Entity> light_sphere;

        OrbitingPointLightElement(
            const ElementRef& parent,
            std::string name,
            glm::vec3 orbit_center,
            float orbit_radius,
            float orbit_tilt_angle,
            float orbit_duration,
            std::shared_ptr<PointLight> light,
            std::shared_ptr<EmissiveEntityRenderer::Entity> light_sphere
        ) :
            SceneElement(parent, std::move(name)),
            orbit_center(orbit_center),
            orbit_radius(orbit_radius),
            orbit_tilt_angle(orbit_tilt_angle),
            orbit_duration(orbit_duration),
            light(std::move(light)),
            light_sphere(std::move(light_sphere)) {}

        static std::unique_ptr<OrbitingPointLightElement> new_default(const SceneContext& scene_context, ElementRef parent);
        static std::unique_ptr<OrbitingPointLightElement> from_json(const SceneContext& scene_context, ElementRef parent, const json& j);

        [[nodiscard]] json into_json() const override;

        [[nodiscard]] glm::vec3 current_position(float time_seconds) const;

        void add_imgui_edit_section(MasterRenderScene& render_scene, const SceneContext& scene_context) override;

        void update_instance_data() override;

        void add_to_render_scene(MasterRenderScene& target_render_scene) override {
            target_render_scene.insert_entity(light_sphere);
            target_render_scene.insert_light(light);
        }

        void remove_from_render_scene(MasterRenderScene& target_render_scene) override {
            target_render_scene.remove_entity(light_sphere);
            target_render_scene.remove_light(light);
        }

        [[nodiscard]] const char* element_type_name() const override;
    };
}

#endif //ORBITING_POINT_LIGHT_ELEMENT_H