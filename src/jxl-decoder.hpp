#pragma once
#include <fstream>
#include <string_view>
#include <vector>

#include <jxl/decode_cxx.h>

#include "util.hpp"

namespace gawl::internal::jxl {
struct JxlImage {
    size_t               width;
    size_t               height;
    std::vector<std::byte> buffer;
};

inline auto decode_jxl(const std::string_view path) -> Result<JxlImage> {
    const auto file_result = read_binary(path);
    if(!file_result) {
        return file_result.as_error();
    }
    const auto& file = file_result.as_value();

    const auto decoder = JxlDecoderMake(NULL);

    if(JxlDecoderSetInput(decoder.get(), std::bit_cast<uint8_t*>(file.data()), file.size()) != JXL_DEC_SUCCESS) {
        return Error("jxl: failed to set input");
    }

    auto              info   = JxlBasicInfo();
    const static auto format = JxlPixelFormat{.num_channels = 4, .data_type = JxlDataType::JXL_TYPE_UINT8, .endianness = JxlEndianness::JXL_NATIVE_ENDIAN, .align = 1};
    auto              buffer = std::vector<std::byte>();

    if(JxlDecoderSubscribeEvents(decoder.get(), JXL_DEC_BASIC_INFO | JXL_DEC_COLOR_ENCODING | JXL_DEC_FULL_IMAGE) != JXL_DEC_SUCCESS) {
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

            buffer.resize(buffer_size);
            if(JxlDecoderSetImageOutBuffer(decoder.get(), &format, buffer.data(), buffer.size()) != JXL_DEC_SUCCESS) {
                return Error("jxl: failed to set output buffer");
            }
        } break;
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
    return JxlImage{info.xsize, info.ysize, std::move(buffer)};
}
} // namespace gawl::jxl
