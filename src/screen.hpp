#pragma once
#include "binder.hpp"
#include "viewport.hpp"

namespace gawl {
class MetaScreen {
  public:
    virtual auto get_scale() const -> double             = 0;
    virtual auto get_viewport() const -> const Viewport& = 0;
    virtual ~MetaScreen() {}
};

class Screen : public MetaScreen {
  public:
    virtual auto prepare() -> impl::FramebufferBinder        = 0;
    virtual auto set_viewport(const Rectangle& rect) -> void = 0;
    virtual auto unset_viewport() -> void                    = 0;
};
} // namespace gawl
