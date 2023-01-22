#pragma once
#include <fmt/core.h>
#include <tl/expected.hpp>

using Result = tl::expected<int, std::string_view>;

inline auto emit_error(std::string_view msg, auto... args) {
    fmt::print(stderr, "error: ", fmt::format(fmt::runtime(msg), args...));
    exit(1);
}