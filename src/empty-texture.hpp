#pragma once
#include "graphic-base.hpp"

namespace gawl {
class EmptyTextureData : public GraphicBase {
  private:
    GLuint frame_buffer;
    std::array<int, 2> size;

  public:
    auto get_size() const -> const std::array<int, 2>&;
    auto get_frame_buffer_name() const -> GLuint ;
    EmptyTextureData(int width, int height);
    ~EmptyTextureData();
};

class EmptyTexture {
  private:
    std::shared_ptr<EmptyTextureData> data;

  public:
    auto get_width(FrameBufferInfo info) const -> int;
    auto get_height(FrameBufferInfo info) const -> int;
    auto draw(FrameBufferInfo info, double x, double y) -> void;
    auto draw_rect(FrameBufferInfo info, Area area) -> void;
    auto draw_fit_rect(FrameBufferInfo info, Area area) -> void;
    auto clear() -> void;
         operator EmptyTextureData*() const;
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
