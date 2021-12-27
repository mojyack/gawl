#pragma once
#include <vector>

namespace gawl {
class GawlWindow;

class GawlApplication {
  private:
    std::vector<GawlWindow*> windows;

    void register_window(GawlWindow* window);

  protected:
    auto unregister_window(const GawlWindow* window) -> bool;
    auto close_all_windows() -> void;

  public:
    template <typename T, typename... A>
    auto open_window(A... args) -> void {
        register_window(new T(*this, args...));
    }
    virtual auto close_window(GawlWindow& window) -> void = 0;
    virtual auto run() -> void                            = 0;
    virtual auto quit() -> void                           = 0;
    virtual auto is_running() const -> bool               = 0;
    virtual ~GawlApplication() {}
};
} // namespace gawl
