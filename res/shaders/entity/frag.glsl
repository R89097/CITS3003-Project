#version 410 core
#include "../common/lights.glsl"

in VertexOut {
    vec3 ws_frag_position;
    vec3 ws_normal;
    vec2 texture_coordinate;
} frag_in;

layout(location = 0) out vec4 out_colour;

// Global Data
uniform float inverse_gamma;

uniform sampler2D diffuse_texture;
uniform sampler2D specular_map_texture;

//moved from vertex
uniform vec3 ws_view_position;

uniform vec3 diffuse_tint;
uniform vec3 specular_tint;
uniform vec3 ambient_tint;
uniform float shininess;


#if NUM_PL > 0
layout (std140) uniform PointLightArray {
    PointLightData point_lights[NUM_PL];
};
#endif

#if NUM_DL > 0
layout (std140) uniform DirectionalLightArray {
    DirectionalLightData directional_lights[NUM_DL];
};
#endif

void main() {
    vec3 ws_view_dir = normalize(ws_view_position - frag_in.ws_frag_position);

    LightCalculatioData light_calculation_data = LightCalculatioData(
        frag_in.ws_frag_position,
        ws_view_dir,
        normalize(frag_in.ws_normal)
    );

    Material material = Material(
        diffuse_tint,
        specular_tint,
        ambient_tint,
        shininess
    );

    LightingResult lighting_result = total_light_calculation(
        light_calculation_data,
        material
        #if NUM_PL > 0
        ,point_lights
        #endif
        #if NUM_DL > 0
        ,directional_lights
        #endif
    );

    vec3 resolved_lighting = resolve_textured_light_calculation(
        lighting_result,
        diffuse_texture,
        specular_map_texture,
        frag_in.texture_coordinate
    );

    float distance_to_camera = length(ws_view_position - frag_in.ws_frag_position);
    float fog_amount = clamp(distance_to_camera / 60.0f, 0.0f, 1.0f);
    vec3 fog_colour = vec3(0.65f, 0.70f, 0.75f);
    resolved_lighting = mix(resolved_lighting, fog_colour, fog_amount * 0.12f);
    out_colour = vec4(resolved_lighting, 1.0f);

    out_colour.rgb = pow(out_colour.rgb, vec3(inverse_gamma));
}