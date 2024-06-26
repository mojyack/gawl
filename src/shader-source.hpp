#pragma once
namespace gawl::impl {
constexpr auto graphic_vertex_shader_source      = R"glsl(
    #version 130
    in vec2  position;
    in vec2  texcoord;
    out vec2 tex_coordinate;
    void main() {
        gl_Position    = vec4(position, 0.0, 1.0);
        tex_coordinate = texcoord;
    }
)glsl";

constexpr auto graphic_fragment_shader_source    = R"glsl(
    #version 130
    in vec2           tex_coordinate;
    uniform sampler2D tex;
    out vec4          color;
    void main() {
        color = texture(tex, tex_coordinate);
    }
)glsl";

constexpr auto textrender_vertex_shader_source   = graphic_vertex_shader_source;

constexpr auto textrender_fragment_shader_source = R"glsl(
    #version 130
    in vec2           tex_coordinate;
    out vec4          color;
    uniform sampler2D tex;
    uniform vec4      text_color;

    void main() {
        vec4 sampled = vec4(1.0, 1.0, 1.0, texture(tex, tex_coordinate).r);
        color        = vec4(text_color) * sampled;
    }
)glsl";

constexpr auto polygon_vertex_shader_source      = R"glsl(
    #version 130
    in vec2 position;
    void main() {
        gl_Position = vec4(position, 0.0, 1.0);
    }
)glsl";

constexpr auto polygon_fragment_shader_source    = R"glsl(
    #version 130
    out vec4     color;
    uniform vec4 polygon_color;
    void main() {
        color = polygon_color;
    }
)glsl";
} // namespace gawl::internal
