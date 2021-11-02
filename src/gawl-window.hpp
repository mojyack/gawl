#pragma once
#include <cstdint>

#include "screen.hpp"
#include "type.hpp"

namespace gawl {
namespace internal {
struct BufferSize {
    std::array<size_t, 2> size  = {800, 600};
    size_t                scale = 1.0;
};
} // namespace internal

class GawlApplication;

struct WindowCreateHint {
    GawlApplication& app;
    size_t           width          = 800;
    size_t           height         = 600;
    const char*      title          = "Window";
    bool             manual_refresh = false;
};

class GawlWindow : public Screen {
    friend class GawlApplication;

  private:
    enum class Status {
        PREPARE,
        READY,
        CLOSE,
    };
    Status               status              = Status::PREPARE;
    bool                 follow_buffer_scale = true;
    double               specified_scale     = 0;
    double               draw_scale          = 1.0;
    bool                 event_driven        = false;
    internal::BufferSize buffer_size;
    std::array<int, 2>   window_size;

  protected:
    GawlApplication& app;

    // used by backend.
    auto on_buffer_resize(size_t width, size_t height, size_t scale) -> void;
    auto is_running() const -> bool;
    auto init_global() -> void;
    auto init_complete() -> void;

    // used by user.
    auto         get_window_size() const -> const std::array<int, 2>&;
    auto         set_follow_buffer_scale(bool flag) -> void;
    auto         get_follow_buffer_scale() const -> bool;
    auto         set_scale(double scale) -> void;
    auto         set_event_driven(bool flag) -> void;
    auto         get_event_driven() const -> bool;
    virtual auto refresh_callback() -> void{};
    virtual auto window_resize_callback() -> void {}
    virtual auto keyboard_callback(uint32_t /* key */, gawl::ButtonState /* state */) -> void {}
    virtual auto pointermove_callback(double /* x */, double /* y */) -> void {}
    virtual auto click_callback(uint32_t /* button */, gawl::ButtonState /* state */) -> void {}
    virtual auto scroll_callback(gawl::WheelAxis /* axis */, double /* value */) -> void {}
    virtual auto close_request_callback() -> void;

  public:
    auto get_scale() const -> double override;
    auto get_size() const -> std::array<std::size_t, 2> override;

    virtual auto refresh() -> void = 0;
    auto         is_close_pending() const -> bool;
    auto         close_window() -> void;
    auto         quit_application() -> void;

    GawlWindow(const GawlWindow&) = delete;
    GawlWindow(GawlWindow&&)      = delete;
    GawlWindow& operator=(const GawlWindow&) = delete;
    GawlWindow& operator=(GawlWindow&&) = delete;

    GawlWindow(const WindowCreateHint& hint);
    virtual ~GawlWindow();
};
} // namespace gawl
