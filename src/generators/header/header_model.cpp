#include <string_view>
#include <string>
#include <yaml-cpp/yaml.h>
#include <fmt/format.h>
#include <fstream>
#include <string>
#include <yaml.h>
#include <utils.h>
#include "header_common.h"
auto header_input(std::string_view command_name, const YAML::Node& input, std::ofstream& output) -> Result<std::string> {
    auto input_typename = fmt::format("input_{}_t", command_name);
    auto resolver = yaml_value_or_else<std::string>(input["resolver"], [] { return std::string{"glap::discard"}; });
    auto validator = yaml_value_or_else<std::string>(input["validator"], [] { return std::string{"glap::discard"}; });
    if (auto maxvalue_config = input["max_number"]; maxvalue_config.IsDefined()) {
        auto maxvalue = maxvalue_config.IsNull() ? "glap::discard" : maxvalue_config.as<std::string>();
        output 
            << fmt::format("using {} = glap::model::Inputs<{}, {}, {}>;", input_typename, maxvalue, resolver, validator)
            << "\n";
    } else {
        output 
            << fmt::format("using {} = glap::model::Input<{}, {}>;", input_typename, resolver, validator)
            << "\n";
    }
    return input_typename;
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
    if (auto input = command.second["input"]; input.IsDefined() && !input.IsNull()) {
        auto input_typename = header_input(names->name, input, output);
        if (!input_typename) {
            return tl::make_unexpected(std::move(input_typename.error()));
        }
        arguments.push_back(std::move(input_typename.value()));
    }
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