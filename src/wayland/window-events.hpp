#pragma once
#include "../buttons.hpp"
#include "../point.hpp"
#include "../variant-buffer.hpp"

namespace gawl::impl {
struct WindowEvents {
    struct Refresh {};

    struct WindowResize {};

    struct Keycode {
        uint32_t          key;
        gawl::ButtonState state;
    };

    struct PointerMove {
        gawl::Point pos;
    };

    struct Click {
        uint32_t          key;
        gawl::ButtonState state;
    };

    struct Scroll {
        gawl::WheelAxis axis;
        double          value;
    };

    struct TouchDown {
        uint32_t    id;
        gawl::Point pos;
    };

    struct TouchUp {
        uint32_t id;
    };

    struct TouchMotion {
        uint32_t    id;
        gawl::Point pos;
    };

    struct CloseRequest {};

    using Queue = VariantBuffer<Refresh, WindowResize, Keycode, PointerMove, Click, Scroll, TouchDown, TouchUp, TouchMotion, CloseRequest>;
};
} // namespace gawl::impl
