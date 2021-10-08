#include "graphic-base.hpp"
#include "error.hpp"
#include "gawl-window.hpp"
#include "global.hpp"
#include "misc.hpp"
#include "type.hpp"

namespace gawl {
extern GlobalVar* global;
namespace {
GLuint  ebo;
GLuint  vbo;
GLfloat vertices[4][4];

auto move_vertices(const Screen* screen, Area area, bool invert) -> void {
    gawl::convert_screen_to_viewport(screen, area);
    vertices[0][0] = area[0];
    vertices[0][1] = area[1 + invert * 2];
    vertices[1][0] = area[2];
    vertices[1][1] = area[1 + invert * 2];
    vertices[2][0] = area[2];
    vertices[2][1] = area[3 - invert * 2];
    vertices[3][0] = area[0];
    vertices[3][1] = area[3 - invert * 2];
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
}
} // namespace
auto init_graphics() -> void {
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), NULL, GL_DYNAMIC_DRAW);

    const static GLuint elements[] = {
        0, 1, 2,
        2, 3, 0};
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

    vertices[0][2] = 0.0;
    vertices[0][3] = 0.0;
    vertices[1][2] = 1.0;
    vertices[1][3] = 0.0;
    vertices[2][2] = 1.0;
    vertices[2][3] = 1.0;
    vertices[3][2] = 0.0;
    vertices[3][3] = 1.0;
}
auto finish_graphics() -> void {
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
}
auto Shader::bind_vao() -> void {
    glBindVertexArray(vao);
}
auto Shader::get_shader() -> GLuint {
    return shader_program;
}
Shader::Shader(const char* const vertex_shader_source, const char* const fragment_shader_source) {
    auto status = GLint();

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &status);
    ASSERT(status == GL_TRUE, "failed to load vertex shader")

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &status);
    ASSERT(status == GL_TRUE, "failed to load fragment shader")

    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &status);
    ASSERT(status == GL_TRUE, "failed to link shaders")
    glBindFragDataLocation(shader_program, 0, "color");

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    const auto pos_attrib = glGetAttribLocation(shader_program, "position");
    glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, 0);
    glEnableVertexAttribArray(pos_attrib);

    const auto tex_attrib = glGetAttribLocation(shader_program, "texcoord");
    glVertexAttribPointer(tex_attrib, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (void*)(sizeof(GLfloat) * 2));
    glEnableVertexAttribArray(tex_attrib);
    glUseProgram(shader_program);

    glBindVertexArray(0);
}
Shader::~Shader() {
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shader_program);
    glDeleteShader(fragment_shader);
    glDeleteShader(vertex_shader);
}
auto GraphicBase::get_texture() const -> GLuint {
    return texture;
}
auto GraphicBase::get_width(const Screen* screen) const -> int {
    return width / screen->get_scale();
}
auto GraphicBase::get_height(const Screen* screen) const -> int {
    return height / screen->get_scale();
}
auto GraphicBase::draw(Screen* screen, const double x, const double y) const -> void {
    draw_rect(screen, {x, y, x + width, y + height});
}
auto GraphicBase::draw_rect(Screen* screen, Area area) const -> void {
    area.magnify(screen->get_scale());
    move_vertices(screen, area, invert_top_bottom);
    type_specific.bind_vao();
    screen->prepare();
    glUseProgram(type_specific.get_shader());
    glBindTexture(GL_TEXTURE_2D, texture);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
auto GraphicBase::draw_fit_rect(Screen* screen, Area area) const -> void {
    draw_rect(screen, calc_fit_rect(area, width, height));
}
GraphicBase::GraphicBase(Shader& type_specific) : type_specific(type_specific) {
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
}
GraphicBase::~GraphicBase() {
    glDeleteTextures(1, &texture);
}
} // namespace gawl
