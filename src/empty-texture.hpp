#pragma once
#include "graphic-base.hpp"

namespace gawl {
class EmptyTextureData : public GraphicBase {
  private:
    GLuint frame_buffer;
    std::array<int, 2> size;

  public:
    const std::array<int, 2>& get_size() const;
    GLuint get_frame_buffer_name() const;
    EmptyTextureData(const int width, const int height);
    ~EmptyTextureData();
};

class EmptyTexture {
  private:
    std::shared_ptr<EmptyTextureData> data;

  public:
    int  get_width(FrameBufferInfo info) const;
    int  get_height(FrameBufferInfo info) const;
    void draw(FrameBufferInfo info, double x, double y);
    void draw_rect(FrameBufferInfo info, Area area);
    void draw_fit_rect(FrameBufferInfo info, Area area);
    void clear();
         operator EmptyTextureData*() const;
         operator GraphicBase*() const;
         operator bool() const;
    EmptyTexture(const EmptyTexture&);
    EmptyTexture(EmptyTexture&&);
    EmptyTexture& operator=(const EmptyTexture&);
    EmptyTexture& operator=(EmptyTexture&&);
    EmptyTexture(){};
    EmptyTexture(const int width, const int height);
};

} // namespace gawl
