#include <yaml-cpp/yaml.h>
#include <fmt/format.h>
#include <yaml.h>
#include <utils.h>
#include <fstream>
#include "header_common.h"

auto get_description(const YAML::Node& config) -> Result<std::string> {
    auto description = config["description"];
    if (!description.IsDefined()) {
        return tl::make_unexpected(fmt::format("program: Missing description"));
    }
    if (auto long_description = config["long_description"]; long_description.IsDefined()) {
        return fmt::format("glap::generators::help::FullDescription<\"{}\", \"{}\">", normalize_string(description.as<std::string>()), normalize_string(long_description.as<std::string>()));
    } else {
        return fmt::format("glap::generators::help::Description<\"{}\">", normalize_string(description.as<std::string>()));
    }
}
auto header_help_argument(std::string_view command_name, YAML::detail::iterator_value command, std::ofstream& output) -> Result<std::string> {
    auto argument_name = command.first.as<std::string>();
    auto argument_type = fmt::format("help_argument_{}_{}", command_name, normalize_string(argument_name));
    auto argument_description = get_description(command.second);
    if (!argument_description) {
        return tl::make_unexpected(argument_description.error());
    }
    output << fmt::format("using {} = glap::generators::help::Argument<\"{}\", {}>;\n", argument_type, argument_name, argument_description.value());
    return argument_type;
}
auto header_help_arguments(std::string_view command_name, const YAML::Node& command, std::ofstream& output) -> Result<std::vector<std::string>> {
    auto arguments = command["arguments"];
    if (!arguments.IsDefined()) {
        return tl::make_unexpected(fmt::format("program: Missing arguments"));
    }
    if (!arguments.IsMap()) {
        return tl::make_unexpected(fmt::format("program: arguments: Must be a map"));
    }
    output << fmt::format("// Help arguments for '{}':\n", command_name);
    std::vector<std::string> argument_types;
    for (auto argument : arguments) {
        auto argument_type = header_help_argument(command_name, argument, output);
        if (!argument_type) {
            return tl::make_unexpected(argument_type.error());
        }
        argument_types.push_back(argument_type.value());
    }
    return argument_types;
}
auto header_help_command(YAML::detail::iterator_value command, std::ofstream& output) -> Result<std::string> {
    auto command_name = command.first.as<std::string>();
    auto command_type = fmt::format("help_command_{}", normalize_string(command_name));
    auto command_description = get_description(command.second);
    if (!command_description) {
        return tl::make_unexpected(command_description.error());
    }
    auto arguments_str = std::string{};
    if (auto arguments = header_help_arguments(command_name, command.second, output); arguments) {
        arguments_str = fmt::format(", {}", fmt::join(arguments.value(), ", "));
    }
    output << fmt::format("// Help command '{}':\n", command_name);
    output << fmt::format("using {} = glap::generators::help::Command<\"{}\", {}{}>;\n", command_type, command_name, command_description.value(), arguments_str);
    return command_type;
}
auto header_help_commands(const YAML::Node& program, std::ofstream& output) -> Result<std::vector<std::string>> {
    auto commands = program["commands"];
    if (!commands.IsDefined()) {
        return tl::make_unexpected(fmt::format("program: Missing commands"));
    }
    if (!commands.IsMap()) {
        return tl::make_unexpected(fmt::format("program: commands: Must be a map"));
    }
    std::vector<std::string> command_types;
    for (auto command : commands) {
        if (auto command_type = header_help_command(command, output); command_type) {
            command_types.push_back(command_type.value());
        } else {
            return tl::make_unexpected(command_type.error());
        }
    }
    return command_types;
}

auto header_help(const YAML::Node& program, std::ofstream& output) -> Result<std::string> {
    const auto help_typename = "help_t";
    auto program_name = program["name"].as<std::string>();
    auto description = get_description(program);
    if (!description) {
        return tl::make_unexpected(description.error());
    }
    output << "// Help\n\n";
    if (auto commands = header_help_commands(program, output); commands) {
        output << fmt::format("using {} = glap::generators::help::Program<\"{}\", {}, {}>;\n", help_typename, program_name, description.value(), fmt::join(commands.value(), ", "));
    } else {
        return tl::make_unexpected(commands.error());
    }
    return help_typename;
}