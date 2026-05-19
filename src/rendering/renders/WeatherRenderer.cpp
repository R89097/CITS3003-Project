#include "WeatherRenderer.h"

#include <iostream>
#include <vector>

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>

namespace {
    GLuint compile_shader(GLenum type, const char* source) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        GLint success = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (success != GL_TRUE) {
            char info_log[1024];
            glGetShaderInfoLog(shader, 1024, nullptr, info_log);
            std::cerr << "Weather shader compile error: " << info_log << std::endl;
        }

        return shader;
    }

    GLuint create_shader_program() {
        const char* vertex_source = R"(
            #version 330 core

            layout(location = 0) in vec3 in_position;
            layout(location = 1) in vec4 in_colour;

            uniform mat4 u_view_projection;

            out vec4 frag_colour;

            void main() {
                gl_Position = u_view_projection * vec4(in_position, 1.0);
                frag_colour = in_colour;
                gl_PointSize = 4.0;
            }
        )";

        const char* fragment_source = R"(
            #version 330 core

            in vec4 frag_colour;
            out vec4 out_colour;

            void main() {
                out_colour = frag_colour;
            }
        )";

        GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_source);
        GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_source);

        GLuint program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);

        GLint success = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (success != GL_TRUE) {
            char info_log[1024];
            glGetProgramInfoLog(program, 1024, nullptr, info_log);
            std::cerr << "Weather shader link error: " << info_log << std::endl;
        }

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);

        return program;
    }
}

Rendering::WeatherRenderer::WeatherRenderer() :
    vao(0),
    vbo(0),
    shader_program(0),
    buffer_vertex_capacity(0)
{
    shader_program = create_shader_program();

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(WeatherVertex), reinterpret_cast<void*>(offsetof(WeatherVertex, position)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(WeatherVertex), reinterpret_cast<void*>(offsetof(WeatherVertex, colour)));

    glBindVertexArray(0);
}

Rendering::WeatherRenderer::~WeatherRenderer() {
    if (vbo != 0) {
        glDeleteBuffers(1, &vbo);
    }

    if (vao != 0) {
        glDeleteVertexArrays(1, &vao);
    }

    if (shader_program != 0) {
        glDeleteProgram(shader_program);
    }
}

void Rendering::WeatherRenderer::render(const RenderScene& scene) {
    glUseProgram(shader_program);

    GLint vp_location = glGetUniformLocation(shader_program, "u_view_projection");
    glUniformMatrix4fv(vp_location, 1, GL_FALSE, glm::value_ptr(scene.view_projection_matrix));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (const Entity& entity : scene.entities) {
        if (!entity.enabled) {
            continue;
        }

        std::vector<WeatherVertex> vertices;
        GLenum primitive_mode = GL_LINES;

        if (entity.weather_type == WeatherType::Rain) {
            vertices = build_rain_vertices(entity);
            primitive_mode = GL_LINES;
        } else {
            vertices = build_snow_vertices(entity);
            primitive_mode = GL_POINTS;
        }

        if (vertices.empty()) {
            continue;
        }

        ensure_buffer_size(vertices.size());
        upload_vertices(vertices);

        glBindVertexArray(vao);
        glDrawArrays(primitive_mode, 0, static_cast<GLsizei>(vertices.size()));
        glBindVertexArray(0);
    }

    glDisable(GL_BLEND);
    glUseProgram(0);
}

void Rendering::WeatherRenderer::ensure_buffer_size(std::size_t vertex_count) {
    if (vertex_count <= buffer_vertex_capacity) {
        return;
    }

    buffer_vertex_capacity = vertex_count;

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(buffer_vertex_capacity * sizeof(WeatherVertex)),
        nullptr,
        GL_DYNAMIC_DRAW
    );
}

void Rendering::WeatherRenderer::upload_vertices(const std::vector<WeatherVertex>& vertices) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(WeatherVertex)),
        vertices.data()
    );
}

std::vector<Rendering::WeatherRenderer::WeatherVertex> Rendering::WeatherRenderer::build_rain_vertices(const Entity& entity) const {
    std::vector<WeatherVertex> vertices;

    glm::vec4 rain_colour(0.65f, 0.8f, 1.0f, 0.55f);

    for (const auto& particle : entity.particles) {
        glm::vec3 direction = glm::normalize(-particle.position);
        glm::vec3 tail = particle.position - direction * entity.rain_length;

        vertices.push_back({particle.position, rain_colour});
        vertices.push_back({tail, rain_colour});
    }

    return vertices;
}

std::vector<Rendering::WeatherRenderer::WeatherVertex> Rendering::WeatherRenderer::build_snow_vertices(const Entity& entity) const {
    std::vector<WeatherVertex> vertices;

    glm::vec4 snow_colour(1.0f, 1.0f, 1.0f, 0.85f);

    for (const auto& particle : entity.particles) {
        vertices.push_back({particle.position, snow_colour});
    }

    return vertices;
}