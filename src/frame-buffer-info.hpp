#pragma once
#include <array>

namespace gawl {
class GawlWindow;
class EmptyTexture;
class EmptyTextureData;
class FrameBufferInfo {
  private:
    const GawlWindow*       window  = nullptr;
    const EmptyTextureData* texture = nullptr;

  public:
    auto get_scale() const -> int;
    auto get_size() const -> std::array<std::size_t, 2>;
    auto prepare() -> void;
    FrameBufferInfo(const GawlWindow* window);
    FrameBufferInfo(const EmptyTexture& texture);
    FrameBufferInfo(std::nullptr_t null);
    FrameBufferInfo(const FrameBufferInfo& other);
};
} // namespace gawl
