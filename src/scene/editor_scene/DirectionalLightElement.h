

#ifndef DIRECTIONAL_LIGHT_ELEMENT_H
#define DIRECTIONAL_LIGHT_ELEMENT_H

#include "SceneElement.h"
#include "scene/SceneContext.h"

namespace EditorScene {
    class DirectionalLightElement : public SceneElement {
    public:
        /// NOTE: Must be unique per element type, as it is used to select generators,
        ///       so if you are creating a new element type make sure to change this to a new unique name.
        static constexpr const char* ELEMENT_TYPE_NAME = "Directional Light";

        // DirectionalLight stores a world direction instead of a world position.
        glm::vec3 direction;
        bool visible = true;
        float visual_scale = 1.0f;

        std::shared_ptr<DirectionalLight> light;
        std::shared_ptr<EmissiveEntityRenderer::Entity> light_visual;

        DirectionalLightElement(const ElementRef& parent, std::string name, glm::vec3 direction, std::shared_ptr<DirectionalLight> light, std::shared_ptr<EmissiveEntityRenderer::Entity> light_visual) :
            SceneElement(parent, std::move(name)), direction(direction), light(std::move(light)), light_visual(std::move(light_visual)) {}

        static std::unique_ptr<DirectionalLightElement> new_default(const SceneContext& scene_context, ElementRef parent);
        static std::unique_ptr<DirectionalLightElement> from_json(const SceneContext& scene_context, ElementRef parent, const json& j);

        [[nodiscard]] json into_json() const override;

        void add_imgui_edit_section(MasterRenderScene& render_scene, const SceneContext& scene_context) override;

        void update_instance_data() override;

        void add_to_render_scene(MasterRenderScene& target_render_scene) override {
            target_render_scene.insert_entity(light_visual);
            target_render_scene.insert_light(light);
        }

        void remove_from_render_scene(MasterRenderScene& target_render_scene) override {
            target_render_scene.remove_entity(light_visual);
            target_render_scene.remove_light(light);
        }

        [[nodiscard]] const char* element_type_name() const override;
    };
}

#endif //DIRECTIONAL_LIGHT_ELEMENT_H