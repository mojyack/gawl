#pragma once
namespace gawl {
constexpr const char* graphic_vertex_shader_source      = R"glsl(
    #version 330 core
    in vec2  position;
    in vec2  texcoord;
    out vec2 tex_coordinate;
    void main() {
        gl_Position    = vec4(position, 0.0, 1.0);
        tex_coordinate = texcoord;
    }
)glsl";
constexpr const char* graphic_fragment_shader_source    = R"glsl(
    #version 330 core
    in vec2           tex_coordinate;
    uniform sampler2D tex;
    out vec4          color;
    void main() {
        color = texture(tex, tex_coordinate);
    }
)glsl";
constexpr const char* textrender_vertex_shader_source   = graphic_vertex_shader_source;
constexpr const char* textrender_fragment_shader_source = R"glsl(
    #version 330 core
    in vec2           tex_coordinate;
    out vec4          color;
    uniform sampler2D tex;
    uniform vec4      text_color;

    void main() {
        vec4 sampled = vec4(1.0, 1.0, 1.0, texture(tex, tex_coordinate).r);
        color        = vec4(text_color) * sampled;
    }
)glsl";
constexpr const char* simple_vertex_shader_source = R"glsl(
    #version 330 core
    in vec2 position;
    void main() {
        gl_Position = vec4(position, 0.0, 1.0);
    }
)glsl";
constexpr const char* simple_fragment_shader_source = R"glsl(
    #version 330 core
    out vec4     color;
    uniform vec4 shape_color;
    void main() {
        color = shape_color;
    }
)glsl";
} // namespace gawl
