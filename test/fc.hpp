#include <fontconfig/fontconfig.h>

#include <string>

namespace fc {
inline auto list_fonts() -> void {
    if(!FcInit()) {
        return;
    }
    const auto config = FcConfigGetCurrent();
    FcConfigSetRescanInterval(config, 0);

    const auto pattern = FcPatternCreate();
    const auto objects = FcObjectSetBuild(FC_FAMILY, FC_STYLE, FC_FILE, nullptr);
    const auto fonts   = FcFontList(config, pattern, objects);
    for(auto i = 0; i < fonts->nfont; i += 1) {
        const auto font = fonts->fonts[i];
        const auto name = std::unique_ptr<FcChar8>(FcNameUnparse(font));
        printf("%s\n", name.get());
    }
    FcFontSetDestroy(fonts);
    FcFini();
}

inline auto find_fontpath_from_name(const char* const name) -> std::string {
    const auto config  = FcInitLoadConfigAndFonts();
    const auto pattern = FcNameParse((const FcChar8*)(name));
    FcConfigSubstitute(config, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);

    auto       result = FcResult();
    const auto font   = FcFontMatch(config, pattern, &result);
    auto       path   = std::string();
    if(font) {
        auto file = (FcChar8*)(nullptr);
        if(FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch) {
            path = std::string((char*)file);
        }
        FcPatternDestroy(font);
    }
    FcPatternDestroy(pattern);
    FcConfigDestroy(config);
    return path;
}
} // namespace fc
