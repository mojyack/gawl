#pragma once
#include <fstream>
#include <string_view>
#include <thread>
#include <vector>

#include <jxl/decode_cxx.h>

#include "util.hpp"

namespace gawl::internal::jxl {
struct Animation {
    uint32_t tps_numerator;
    uint32_t tps_denominator;
    uint32_t loops;
};

struct Frame {
    std::vector<std::byte> buffer;
    uint32_t               duration;
};

struct JxlImage {
    uint32_t           width;
    uint32_t           height;
    std::vector<Frame> frames;
    Animation          animation;
    bool               have_animation;
};

class ParallelRunner {
  private:
    uint32_t threads;

  public:
    static auto entry(void* const runner_opaque, void* const jpegxl_opaque, const JxlParallelRunInit init, const JxlParallelRunFunction func, const uint32_t start_range, const uint32_t end_range) -> JxlParallelRetCode {
        auto& self = *std::bit_cast<ParallelRunner*>(runner_opaque);

        if(const auto e = init(jpegxl_opaque, self.threads); e != 0) {
            return e;
        }

        auto value_index = std::atomic_uint32_t(start_range);
        auto workers     = std::vector<std::thread>(self.threads);

        for(auto w = size_t(0); w < workers.size(); w += 1) {
            workers[w] = std::thread([&value_index, jpegxl_opaque, func, end_range, w]() {
                loop:
                    const auto value = value_index.fetch_add(1);
                    if(value >= end_range) {
                        return;
                    }
                    func(jpegxl_opaque, value, w);
                    goto loop;
            });
        }

        for(auto& w : workers) {
            w.join();
        }

        return 0;
    }

    ParallelRunner(const uint32_t threads) : threads(threads) {}
};

inline auto decode_jxl(const char* const path, const uint32_t threads = std::thread::hardware_concurrency()) -> Result<JxlImage> {
    const auto file_result = read_binary(path);
    if(!file_result) {
        return file_result.as_error();
    }
    const auto& file = file_result.as_value();

    const auto decoder = JxlDecoderMake(NULL);

    if(JxlDecoderSetInput(decoder.get(), std::bit_cast<uint8_t*>(file.data()), file.size()) != JXL_DEC_SUCCESS) {
        return Error("jxl: failed to set input");
    }

    auto runner = ParallelRunner(threads);

    if(JxlDecoderSetParallelRunner(decoder.get(), ParallelRunner::entry, &runner) != JXL_DEC_SUCCESS) {
        return Error("jxl: failed to set runner");
    }

    auto              info   = JxlBasicInfo();
    const static auto format = JxlPixelFormat{.num_channels = 4, .data_type = JxlDataType::JXL_TYPE_UINT8, .endianness = JxlEndianness::JXL_NATIVE_ENDIAN, .align = 1};
    auto              frames = std::vector<Frame>();
    auto              frame  = (Frame*)(nullptr);

    if(JxlDecoderSubscribeEvents(decoder.get(), JXL_DEC_BASIC_INFO | JXL_DEC_COLOR_ENCODING | JXL_DEC_FRAME | JXL_DEC_FULL_IMAGE) != JXL_DEC_SUCCESS) {
        return Error("jxl: failed to subscribe events");
    }

    while(true) {
        switch(JxlDecoderProcessInput(decoder.get())) {
        case JXL_DEC_ERROR:
            return Error("jxl: decoder error");
        case JXL_DEC_NEED_MORE_INPUT:
            return Error("jxl: no more inputs");
        case JXL_DEC_BASIC_INFO:
            if(JxlDecoderGetBasicInfo(decoder.get(), &info) != JXL_DEC_SUCCESS) {
                return Error("jxl: failed to get basic info");
            }
            break;
        case JXL_DEC_COLOR_ENCODING:
            continue;
        case JXL_DEC_NEED_IMAGE_OUT_BUFFER: {
            auto buffer_size = size_t();
            if(JxlDecoderImageOutBufferSize(decoder.get(), &format, &buffer_size)) {
                return Error("jxl: failed to get output buffer size");
            }

            auto& buffer = frame->buffer;
            buffer.resize(buffer_size);
            if(JxlDecoderSetImageOutBuffer(decoder.get(), &format, buffer.data(), buffer.size()) != JXL_DEC_SUCCESS) {
                return Error("jxl: failed to set output buffer");
            }
        } break;
        case JXL_DEC_FRAME:
            frame = &frames.emplace_back();
            if(info.have_animation) {
                auto header = JxlFrameHeader();
                if(JxlDecoderGetFrameHeader(decoder.get(), &header) != JXL_DEC_SUCCESS) {
                    return Error("jxl: failed to get frame header");
                }
                frame->duration = header.duration;
            } else {
                frame->duration = 0;
            }
            break;
        case JXL_DEC_FULL_IMAGE:
            break;
        case JXL_DEC_SUCCESS:
            goto finish;
            break;
        default:
            return Error("jxl: unknown state");
        }
    }

finish:
    auto image   = JxlImage();
    image.width  = info.xsize;
    image.height = info.ysize;
    image.frames = std::move(frames);
    if(info.have_animation) {
        const auto& anim = info.animation;

        image.have_animation = true;
        image.animation      = {.tps_numerator = anim.tps_numerator, .tps_denominator = anim.tps_denominator, .loops = anim.num_loops};
    } else {
        image.have_animation = false;
    }

    return image;
}
} // namespace gawl::internal::jxl
