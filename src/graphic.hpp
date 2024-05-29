#pragma once
#include "graphic-base.hpp"
#include "pixelbuffer.hpp"

namespace gawl {
class Graphic : public impl::GraphicBase {
  public:
    auto update_texture(const PixelBuffer& buffer, std::optional<std::array<int, 4>> crop = std::nullopt) -> void;

    Graphic() {}
    Graphic(const PixelBuffer& buffer, std::optional<std::array<int, 4>> crop = std::nullopt);
};

// static checks
static_assert(std::movable<Graphic>);
static_assert(!std::copyable<Graphic>);
} // namespace gawl
