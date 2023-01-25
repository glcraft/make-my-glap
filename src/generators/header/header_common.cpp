#include <string_view>
#include <string>
#include <glap/core/utf8.h>
#include <tl/expected.hpp>
#include <yaml-cpp/yaml.h>
#include <utils.h>
#include "header_common.h"

auto check_name(std::string_view name) -> Result<int> {
    if (name.empty()) {
        return tl::make_unexpected(std::string{"Name is empty"});
    }
    if (std::isdigit(name[0])) {
        return tl::make_unexpected(fmt::format("'{}' : Name starts with a digit", name));
    }
    for (auto c : name) {
        if (!std::isalnum(c) && c != '_') {
            return tl::make_unexpected(fmt::format("'{}' : Name contains invalid character '{}'", name, c));
        }
    }
    return 0;
}
auto get_shortname(YAML::Node shortname) -> Result<std::string> {
    std::string shortname_str{"glap::discard"};
    if (shortname.IsDefined() && !shortname.IsNull()) {
        shortname_str = shortname.as<std::string>();
        auto length = glap::utils::uni::utf8_length(shortname_str);
        if (!length) {
            return tl::make_unexpected(std::string{"Shortname is not valid utf8"});
        } else if (length.value() > 1) {
            return tl::make_unexpected(std::string{"Shortname is longer than 1 character"});
        }
        shortname_str = fmt::format("'{}'", shortname_str);
    }
    return shortname_str;
}
auto get_names(YAML::Node name, YAML::Node config) -> Result<Names> {
    auto shortname = get_shortname(config["shortname"]);
    if (!shortname) {
        return tl::make_unexpected(std::move(shortname.error()));
    }
    auto name_str = name.as<std::string>();
    if (auto result = check_name(name_str); !result) {
        return tl::make_unexpected(std::move(result.error()));
    }
    auto glapnames = fmt::format("glap::Names<\"{}\", {}>", name_str, std::move(shortname.value()));
    return Names{std::move(name_str), std::move(glapnames)};
}

// count the number of white characters in a string
auto count_white(std::string_view name) -> size_t {
    size_t count = 0;
    for (auto c : name) {
        if (std::isspace(c))
            ++count;
    }
    return count;
}
// replace every white character with its escaped version
auto normalize_string(std::string_view text) -> std::string {
    std::string result;
    auto count = count_white(text);
    if (count == 0)
        return std::string{text};
    result.reserve(text.size()+count);
    auto previous = text.begin();
    for (auto it = text.begin(); it != text.end(); ++it) {
        auto c = *it;
        if (c != ' ' && std::isspace(c)) {
            if (previous != it)
                result.append(previous, it);
            switch (c) {
                case '\n':
                    result.append("\\n");
                    break;
                case '\t':
                    result.append("\\t");
                    break;
                case '\r':
                    result.append("\\r");
                    break;
                case '\v':
                    result.append("\\v");
                    break;
                case '\f':
                    result.append("\\f");
                    break;
                default:
                    result.push_back(c);
            }
            previous = std::next(it);
        }
    }
    if (previous != text.end())
        result.append(previous, text.end());
    return result;
}