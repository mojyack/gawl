#pragma once
#include <exception>
#include <iostream>
#include <sstream>

namespace gawl {
template <class... Args>
[[noreturn]] auto panic(Args... args) -> void {
    auto ss = std::stringstream();
    (ss << ... << args) << std::endl;
    throw std::runtime_error(ss.str());
}

template <class... Args>
auto warn(Args... args) -> void {
    (std::cerr << ... << args) << std::endl;
}

template <class... Args>
auto print(Args... args) -> void {
    (std::cout << ... << args) << std::endl;
}
} // namespace gawl
