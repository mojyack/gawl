#include <jxl/decode_cxx.h>

#include "jxl-decoder.hpp"
#include "macros/assert.hpp"

#define CUTIL_NS gawl
#include "macros/unwrap.hpp"
#include "util/file-io.hpp"
#undef CUTIL_NS

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

        for(auto w = 0uz; w < workers.size(); w += 1) {
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
    unwrap(file, read_file(path));

    const auto decoder = JxlDecoderMake(NULL);

    ensure(JxlDecoderSetInput(decoder.get(), std::bit_cast<uint8_t*>(file.data()), file.size()) == JXL_DEC_SUCCESS);
    auto runner = ParallelRunner(threads);
    ensure(JxlDecoderSetParallelRunner(decoder.get(), ParallelRunner::entry, &runner) == JXL_DEC_SUCCESS);
    ensure(JxlDecoderSubscribeEvents(decoder.get(), JXL_DEC_BASIC_INFO | JXL_DEC_COLOR_ENCODING | JXL_DEC_FRAME | JXL_DEC_FULL_IMAGE) == JXL_DEC_SUCCESS);

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
            bail("decoder error");
        case JXL_DEC_NEED_MORE_INPUT:
            bail("no more inputs");
        case JXL_DEC_BASIC_INFO:
            ensure(JxlDecoderGetBasicInfo(decoder.get(), &info) == JXL_DEC_SUCCESS);
            break;
        case JXL_DEC_COLOR_ENCODING:
            continue;
        case JXL_DEC_NEED_IMAGE_OUT_BUFFER: {
            auto buffer_size = size_t();
            ensure(JxlDecoderImageOutBufferSize(decoder.get(), &format, &buffer_size) == JXL_DEC_SUCCESS);
            auto& buffer = frame->buffer;
            buffer.resize(buffer_size);
            ensure(JxlDecoderSetImageOutBuffer(decoder.get(), &format, buffer.data(), buffer.size()) == JXL_DEC_SUCCESS);
        } break;
        case JXL_DEC_FRAME:
            frame = &frames.emplace_back();
            if(info.have_animation) {
                auto header = JxlFrameHeader();
                ensure(JxlDecoderGetFrameHeader(decoder.get(), &header) == JXL_DEC_SUCCESS);
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
            bail("unknown state");
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
