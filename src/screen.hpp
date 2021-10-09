#pragma once
#include <array>

namespace gawl {
class Screen {
  public:
    virtual auto get_scale() const -> double                    = 0;
    virtual auto get_size() const -> std::array<std::size_t, 2> = 0;
    virtual auto prepare() -> void                              = 0;
    virtual ~Screen() {}
};

class NullScreen : Screen {
  public:
    auto get_scale() const -> double override;
    auto get_size() const -> std::array<std::size_t, 2> override;
    auto prepare() -> void override;
};
} // namespace gawl
