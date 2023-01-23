#include "glap/core/utf8.h"
#include <__concepts/convertible_to.h>
#include <glap/config.h>
#include <tl/expected.hpp>
#include <string_view>
#include <span>
#include <fmt/format.h>
#include <type_traits>
#include <yaml.h>
#include <utils.h>
#include <generators.h>

auto check_name(std::string_view name) -> Result<int> {
    if (name.empty()) {
        return tl::make_unexpected("Name is empty");
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
auto header_command(YAML::detail::iterator_value command, std::ofstream& output) -> Result<std::string> {
    auto name = command.first.as<std::string>();
    if (auto check_result = check_name(name); !check_result) {
        return tl::make_unexpected(check_result.error());
    }
    auto command_config = command.second;
    auto shortname = command_config["shortname"];
    std::string shortname_str{"glap::discard"};
    if (shortname.IsDefined() && !shortname.IsNull()) {
        shortname_str = shortname.as<std::string>();
        auto length = glap::utils::uni::utf8_length(shortname_str);
        if (!length) {
            return tl::make_unexpected(fmt::format("'{}' : Shortname is not valid utf8", name));
        } else if (length.value() > 1) {
            return tl::make_unexpected(fmt::format("'{}' : Shortname is longer than 1 character", name));
        }
        shortname_str = fmt::format("'{}'", shortname_str);
    }
    auto command_typename = fmt::format("command_{}_t", name);
    output << 
        fmt::format("using {} = glap::model::Command<glap::Names<\"{}\", {}>>;", command_typename, name, shortname_str)
        << "\n";
    return command_typename;
}
auto header_commands(const YAML::Node& config, std::ofstream& output) -> Result<std::vector<std::string>> {
    auto commands = config["commands"];
    if (!commands.IsDefined()) {
        return tl::make_unexpected("Commands are not defined");
    }
    auto result = std::vector<std::string>{};
    for (auto command : commands) {
        auto command_result = header_command(command, output);
        if (!command_result) {
            return tl::make_unexpected(command_result.error());
        }
        result.push_back(std::move(command_result.value()));
    }
    output << "\n";
    return result;
}
auto header_program(const YAML::Node& config, std::ofstream& output) -> Result<int> {
    auto name = config["name"];
    if (!name.IsDefined()) {
        return tl::make_unexpected("Program name is not defined");
    }
    auto default_command = yaml_value_or(config["default_command"], false) ? "glap::model::DefaultCommand::FirstDefined" : "glap::model::DefaultCommand::None";
    auto commands_result = header_commands(config, output);
    if (!commands_result) {
        return tl::make_unexpected(commands_result.error());
    }
    auto commands = std::move(commands_result.value());
    auto commands_str = join_strings(commands, ", ");
    if (! commands_str.empty()) {
        output 
            << fmt::format("using program_t = glap::model::Program<\"{}\", {}, {}>;", name.as<std::string>(), default_command, commands_str) 
            << "\n\n";
    } else {
        output 
            << fmt::format("using program_t = glap::model::Program<\"{}\", {}>;", name.as<std::string>(), default_command)
            << "\n\n";
    }
    return 0;
}
auto generate_header(YAML::Node config, std::ofstream output) -> Result<int> {
    output << "#pragma once\n\n";
#ifdef GLAP_USE_MODULE
    output << "import glap;\n\n";
#else 
    output << "#include <glap/glap.h>\n\n";
#endif
    return header_program(config["program"], output);
}

//using program_t = glap::model::Program<"myprogram", 
//glap::model::DefaultCommand::FirstDefined, 
//command1_t, 
//command2_t>;