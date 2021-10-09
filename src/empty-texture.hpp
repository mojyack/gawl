#pragma once
#include "graphic-base.hpp"
#include "screen.hpp"

namespace gawl {
namespace internal {
class EmptyTextureData : public GraphicBase {
  private:
    GLuint                frame_buffer;
    std::array<size_t, 2> size;

  public:
    auto get_size() const -> const std::array<size_t, 2>&;
    auto get_frame_buffer_name() const -> GLuint;
    EmptyTextureData(size_t width, size_t height);
    ~EmptyTextureData();
};
} // namespace internal

class EmptyTexture : public Screen {
  private:
    std::shared_ptr<internal::EmptyTextureData> data;

  public:
    auto get_scale() const -> double override;
    auto get_size() const -> std::array<std::size_t, 2> override;
    auto prepare() -> void override;

    auto get_width(const Screen* screen) const -> int;
    auto get_height(const Screen* screen) const -> int;
    auto draw(Screen* screen, const Point& point) -> void;
    auto draw_rect(Screen* screen, const Rectangle& rect) -> void;
    auto draw_fit_rect(Screen* screen, const Rectangle& rect) -> void;
    auto clear() -> void;
         operator internal::EmptyTextureData*() const;
         operator GraphicBase*() const;
         operator bool() const;
    EmptyTexture(const EmptyTexture&);
    EmptyTexture(EmptyTexture&&);
    EmptyTexture& operator=(const EmptyTexture&);
    EmptyTexture& operator=(EmptyTexture&&);
    EmptyTexture(){};
    EmptyTexture(int width, int height);
};
} // namespace gawl
