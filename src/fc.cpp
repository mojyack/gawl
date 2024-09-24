#include <optional>
#include <string>

#include <fontconfig/fontconfig.h>

#include "macros/assert.hpp"
#include "macros/autoptr.hpp"

namespace {
declare_autoptr(FcConfig, FcConfig, FcConfigDestroy);
declare_autoptr(FcPattern, FcPattern, FcPatternDestroy);
} // namespace

namespace gawl {
auto find_fontpath_from_name(const char* const name) -> std::optional<std::string> {
    const auto config  = AutoFcConfig(FcInitLoadConfigAndFonts());
    const auto pattern = AutoFcPattern(FcNameParse((const FcChar8*)(name)));
    FcConfigSubstitute(config.get(), pattern.get(), FcMatchPattern);
    FcDefaultSubstitute(pattern.get());

    auto       result = FcResult();
    const auto font   = AutoFcPattern(FcFontMatch(config.get(), pattern.get(), &result));
    ensure(font);
    auto file = (FcChar8*)(nullptr);
    ensure(FcPatternGetString(font.get(), FC_FILE, 0, &file) == FcResultMatch);
    return (char*)file;
}
} // namespace gawl
