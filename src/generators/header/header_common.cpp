#include <glap/core/utf8.h>
#include <utils.h>
#include <tl/expected.hpp>
#include <string_view>
#include <yaml-cpp/yaml.h>
#include <string>

struct Names {
    std::string name;
    std::string glapnames;
};

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
