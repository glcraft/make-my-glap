#pragma once
#include <fmt/core.h>
#include <tl/expected.hpp>

template <typename T>
using Result = tl::expected<T, std::string>;

inline auto emit_error(std::string_view msg) {
    fmt::print(stderr, "error: {}\n", msg);
    exit(1);
}
template <typename Cont>
    requires std::ranges::range<Cont> && std::convertible_to<std::ranges::range_value_t<Cont>, std::string_view>
auto join_strings(const Cont& strings, std::string_view separator) -> std::string {
    std::string result;
    for (std::string_view str : strings) {
        result.append(str);
        result.append(separator);
    }
    result.erase(result.size() - separator.size());
    return result;
}