#include "screen.hpp"
#include "error.hpp"

namespace gawl {
auto NullScreen::get_scale() const -> int {
    return 1;
}
auto NullScreen::get_size() const -> std::array<std::size_t, 2> {
    return {0, 0};
}
auto NullScreen::prepare() -> void {
    panic("attempt to draw to null-screen");
    return;
}
}
