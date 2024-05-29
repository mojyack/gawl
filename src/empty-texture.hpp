#pragma once
#include "graphic-base.hpp"
#include "screen.hpp"

namespace gawl {
class EmptyTexture : public impl::GraphicBase,
                     public Screen {
  private:
    GLuint   frame_buffer;
    Viewport viewport;

  public:
    auto get_scale() const -> double override;
    auto set_viewport(const gawl::Rectangle& region) -> void override;
    auto unset_viewport() -> void override;
    auto get_viewport() const -> const Viewport& override;
    auto prepare() -> impl::FramebufferBinder override;

    EmptyTexture(int width, int height);
    ~EmptyTexture();
};

} // namespace gawl
