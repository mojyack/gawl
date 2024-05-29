#include <optional>
#include <string>

#include <fontconfig/fontconfig.h>

namespace gawl {
auto find_fontpath_from_name(const char* const name) -> std::optional<std::string> {
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
        return std::nullopt;
    } else {
        return path;
    }
}
} // namespace gawl
