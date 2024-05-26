#include <jxl/decode_cxx.h>

#include "jxl-decoder.hpp"
#include "macros/assert.hpp"
#include "util.hpp"

namespace gawl::impl::jxl {
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

auto decode_jxl(const char* const path, const uint32_t threads) -> std::optional<JxlImage> {
    const auto file_r = read_binary(path);
    assert_o(file_r, file_r.as_error().cstr());
    const auto& file = file_r.as_value();

    const auto decoder = JxlDecoderMake(NULL);

    assert_o(JxlDecoderSetInput(decoder.get(), std::bit_cast<uint8_t*>(file.data()), file.size()) == JXL_DEC_SUCCESS);
    auto runner = ParallelRunner(threads);
    assert_o(JxlDecoderSetParallelRunner(decoder.get(), ParallelRunner::entry, &runner) == JXL_DEC_SUCCESS);
    assert_o(JxlDecoderSubscribeEvents(decoder.get(), JXL_DEC_BASIC_INFO | JXL_DEC_COLOR_ENCODING | JXL_DEC_FRAME | JXL_DEC_FULL_IMAGE) == JXL_DEC_SUCCESS);

    static const auto format = JxlPixelFormat{
        .num_channels = 4,
        .data_type    = JxlDataType::JXL_TYPE_UINT8,
        .endianness   = JxlEndianness::JXL_NATIVE_ENDIAN,
        .align        = 1,
    };

    auto info   = JxlBasicInfo();
    auto frames = std::vector<Frame>();
    auto frame  = (Frame*)(nullptr);

    while(true) {
        switch(JxlDecoderProcessInput(decoder.get())) {
        case JXL_DEC_ERROR:
            WARN("decoder error");
            return std::nullopt;
        case JXL_DEC_NEED_MORE_INPUT:
            WARN("no more inputs");
            return std::nullopt;
        case JXL_DEC_BASIC_INFO:
            assert_o(JxlDecoderGetBasicInfo(decoder.get(), &info) == JXL_DEC_SUCCESS);
            break;
        case JXL_DEC_COLOR_ENCODING:
            continue;
        case JXL_DEC_NEED_IMAGE_OUT_BUFFER: {
            auto buffer_size = size_t();
            assert_o(JxlDecoderImageOutBufferSize(decoder.get(), &format, &buffer_size) == JXL_DEC_SUCCESS);
            auto& buffer = frame->buffer;
            buffer.resize(buffer_size);
            assert_o(JxlDecoderSetImageOutBuffer(decoder.get(), &format, buffer.data(), buffer.size()) == JXL_DEC_SUCCESS);
        } break;
        case JXL_DEC_FRAME:
            frame = &frames.emplace_back();
            if(info.have_animation) {
                auto header = JxlFrameHeader();
                assert_o(JxlDecoderGetFrameHeader(decoder.get(), &header) == JXL_DEC_SUCCESS);
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
            WARN("unknown state");
            return std::nullopt;
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
} // namespace gawl::impl::jxl
