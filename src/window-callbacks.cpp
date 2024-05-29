#include "application.hpp"

namespace gawl {
auto WindowCallbacks::close() -> void {
    application->close_window(window);
}
} // namespace gawl
