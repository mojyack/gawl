#pragma once
#include <vector>

#include "shader.hpp"

namespace gawl::impl {
class PolygonShader : public Shader {
  private:
    size_t vbo_capacity = 0;

  public:
    auto write_buffer(const std::vector<GLfloat>& buffer) -> void;
    auto init() -> bool;
};
} // namespace gawl::impl
