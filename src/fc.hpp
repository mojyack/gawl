#pragma once
#include <string>

#include <fontconfig/fontconfig.h>

#include "util.hpp"

namespace gawl {
inline auto find_fontpath_from_name(const char* const name) -> Result<std::string> {
    const auto config  = FcInitLoadConfigAndFonts();
    const auto pattern = FcNameParse((const FcChar8*)(name));
    FcConfigSubstitute(config, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);

    auto result = FcResult();
    auto path   = std::string();
    if(const auto font = FcFontMatch(config, pattern, &result)) {
        auto file = (FcChar8*)(nullptr);
        if(FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch) {
            path = std::string((char*)file);
        }
        FcPatternDestroy(font);
    }
    FcPatternDestroy(pattern);
    FcConfigDestroy(config);
    if(path.empty()) {
        return Error("failed to find font");
    } else {
        return path;
    }
}
} // namespace gawl
