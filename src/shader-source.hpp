#pragma once
namespace gawl {
constexpr const char* graphic_vertex_shader_source      = R"glsl(
            #version 330 core
            in vec2 position;
            in vec2 texcoord;
            out vec2 Texcoord;
            void main(){
                gl_Position = vec4(position, 0.0, 1.0);
                Texcoord = texcoord;
            }
        )glsl";
constexpr const char* graphic_fragment_shader_source    = R"glsl(
            #version 330 core
            in vec2 Texcoord;
            uniform sampler2D tex;
            out vec4 outColor;
            void main() {
                outColor = texture(tex, Texcoord);
            }
        )glsl";
constexpr const char* textrender_vertex_shader_source   = R"glsl(
            #version 330 core
            in vec2 position;
            in vec2 texcoord;
            out vec2 Texcoord;
            void main() {
                gl_Position = vec4(position, 0.0, 1.0);
                Texcoord = texcoord;
            }  
        )glsl";
constexpr const char* textrender_fragment_shader_source = R"glsl(
            #version 330 core
            in vec2  Texcoord;
            out vec4 outColor;

            uniform sampler2D tex;
            uniform vec4      textColor;

            void main() {
                vec4 sampled = vec4(1.0, 1.0, 1.0, texture(tex, Texcoord).r);
                outColor     = vec4(textColor) * sampled;
            }
        )glsl";
} // namespace gawl
