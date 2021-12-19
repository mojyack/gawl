#include "screen.hpp"
#include "error.hpp"

namespace gawl {
const NullScreen nullscreen_;
const NullScreen* const nullscreen = &nullscreen_;
auto NullScreen::get_scale() const -> double {
    return 1.0;
}
auto NullScreen::get_size() const -> std::array<std::size_t, 2> {
    return {0, 0};
}
auto NullScreen::prepare() -> internal::FramebufferBinder {
    panic("attempt to draw to null-screen");
    return internal::FramebufferBinder();
}
} // namespace gawl
