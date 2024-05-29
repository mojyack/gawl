#pragma once
#include <optional>
#include <string>

namespace gawl {
auto find_fontpath_from_name(const char* const name) -> std::optional<std::string>;
}
