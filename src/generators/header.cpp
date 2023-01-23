#include "glap/core/utf8.h"
#include <glap/config.h>
#include <tl/expected.hpp>
#include <string_view>
#include <span>
#include <fmt/format.h>
#include <type_traits>
#include <vector>
#include <concepts>
#include <yaml.h>
#include <utils.h>
#include <generators.h>

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
auto header_parameter(std::string_view command_name, YAML::detail::iterator_value argument, std::ofstream& output) -> Result<std::string> {
    auto names = get_names(argument.first, argument.second);
    if (!names) {
        return tl::make_unexpected(std::move(names.error()));
    }
    auto argument_typename = fmt::format("parameter_{}_{}_t", command_name, names->name);
    auto resolver = yaml_value_or_else<std::string>(argument.second["resolver"], [] { return std::string{"glap::discard"}; });
    auto validator = yaml_value_or_else<std::string>(argument.second["validator"], [] { return std::string{"glap::discard"}; });
    if (auto maxvalue_config = argument.second["max_number"]; maxvalue_config.IsDefined()) {
        auto maxvalue = maxvalue_config.IsNull() ? "glap::discard" : maxvalue_config.as<std::string>();
        output 
            << fmt::format("using {} = glap::model::Parameters<{}, {}, {}, {}>;", argument_typename, names->glapnames, maxvalue, resolver, validator)
            << "\n";
    } else {
        output 
            << fmt::format("using {} = glap::model::Parameter<{}, {}, {}>;", argument_typename, names->glapnames, resolver, validator)
            << "\n";
    }
    return argument_typename;
}
auto header_flag(std::string_view command_name, YAML::detail::iterator_value argument, std::ofstream& output) -> Result<std::string> {
    auto names = get_names(argument.first, argument.second);
    if (!names) {
        return tl::make_unexpected(std::move(names.error()));
    }
    auto argument_typename = fmt::format("flag_{}_{}_t", command_name, names->name);
    output << 
        fmt::format("using {} = glap::model::Flag<{}>;", argument_typename, names->glapnames)
        << "\n";
    return argument_typename;
}
auto header_argument(std::string_view command_name, YAML::detail::iterator_value argument, std::ofstream& output) -> Result<std::string> {
    auto name = argument.first.as<std::string>();
    auto argument_config = argument.second;
    auto type = argument_config["type"];
    auto result = [&] () -> Result<std::string> {
        if (!type.IsDefined()) {
            return tl::make_unexpected(fmt::format("'{}' : Missing type [flag or parameter]", name));
        } else if (type.as<std::string>() == "flag") {
            return header_flag(command_name, std::move(argument), output);
        } else if (type.as<std::string>() == "parameter") {
            return header_parameter(command_name, std::move(argument), output);
        } else {
            return tl::make_unexpected(fmt::format("'{}' : Unknown type '{}'", name, type.as<std::string>()));
        }
    }();
    if (!result) {
        return tl::make_unexpected(std::move(result.error()));
    }
    return std::move(result.value());
}
auto header_arguments(std::string_view command_name, const YAML::Node& config, std::ofstream& output) -> Result<std::vector<std::string>> {
    auto arguments = config["arguments"];
    if (!arguments.IsDefined() || arguments.IsNull()) {
        return std::vector<std::string>{};
    }
    if (!arguments.IsMap()) {
        return tl::make_unexpected(fmt::format("'{}' : Arguments is not a map", command_name));
    }
    output << fmt::format("// Arguments for '{}'\n", command_name);
    auto result = std::vector<std::string>{};
    for (auto argument : arguments) {
        auto argument_result = header_argument(command_name, argument, output);
        if (!argument_result) {
            return tl::make_unexpected(std::move(argument_result.error()));
        }
        result.push_back(std::move(argument_result.value()));
    }
    output << "\n";
    return result;
}
auto header_command(YAML::detail::iterator_value command, std::ofstream& output) -> Result<std::string> {
    auto names = get_names(command.first, command.second);
    if (!names) {
        return tl::make_unexpected(std::move(names.error()));
    }
    auto command_config = command.second;
    auto command_typename = fmt::format("command_{}_t", names->name);
    auto arguments_result = header_arguments(names->name, command_config, output);
    if (!arguments_result) {
        return tl::make_unexpected(std::move(arguments_result.error()));
    }
    auto arguments = std::move(arguments_result.value());
    auto arguments_str = join_strings(arguments, ", ");
    if (!arguments.empty()) {
        output << 
            fmt::format("using {} = glap::model::Command<{}, {}>;", command_typename, names->glapnames, arguments_str)
            << "\n";
    } else {
        output << 
            fmt::format("using {} = glap::model::Command<{}>;", command_typename, names->glapnames)
            << "\n";
    }
    return command_typename;
}
auto header_commands(const YAML::Node& config, std::ofstream& output) -> Result<std::vector<std::string>> {
    auto commands = config["commands"];
    if (!commands.IsDefined()) {
        return tl::make_unexpected(std::string{"Commands are not defined"});
    }
    auto result = std::vector<std::string>{};
    for (auto command : commands) {
        auto command_result = header_command(command, output);
        if (!command_result) {
            return tl::make_unexpected(std::move(command_result.error()));
        }
        result.push_back(std::move(std::move(command_result.value())));
    }
    output << "\n";
    return result;
}
auto header_program(const YAML::Node& config, std::ofstream& output) -> Result<int> {
    auto name = config["name"];
    if (!name.IsDefined()) {
        return tl::make_unexpected(std::string{"Program name is not defined"});
    }
    auto default_command = yaml_value_or(config["default_command"], false) ? "glap::model::DefaultCommand::FirstDefined" : "glap::model::DefaultCommand::None";
    auto commands_result = header_commands(config, output);
    if (!commands_result) {
        return tl::make_unexpected(std::move(commands_result.error()));
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