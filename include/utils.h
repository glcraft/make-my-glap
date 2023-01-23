#pragma once
#include <fmt/core.h>
#include <tl/expected.hpp>

template <typename T>
using Result = tl::expected<T, std::string>;

inline auto emit_error(std::string_view msg, auto... args) {
    fmt::print(stderr, "error: ", fmt::format(fmt::runtime(msg), args...));
    exit(1);
}