#pragma once
#include <fmt/core.h>
#include <tl/expected.hpp>

template <typename T>
using Result = tl::expected<T, std::string>;

inline auto emit_error(std::string_view msg) {
    fmt::print(stderr, "error: {}\n", msg);
    exit(1);
}