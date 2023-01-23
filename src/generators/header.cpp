#include <tl/expected.hpp>
#include <string_view>
#include <fmt/format.h>
#include <type_traits>
#include <yaml.h>
#include <utils.h>
#include <generators.h>

auto header_program(const YAML::Node& config, std::ofstream& output) -> Result {
    auto name = config["name"];
    if (!name.IsDefined()) {
        return tl::make_unexpected("Program name is not defined");
    }
    auto default_command = yaml_value_or(config["default_command"], false);
    auto default_command_str = default_command ? "glap::model::DefaultCommand::FirstDefined" : "glap::model::DefaultCommand::None";

    output 
        << fmt::format("using program_t = glap::model::Program<\"{}\", {}>;", name.as<std::string_view>(), default_command_str)
        << "\n\n";
    return 0;
}
auto generate_header(YAML::Node config, std::ofstream output) -> Result<int> {
    output << "#pragma once\n\n";

    header_program(config["program"], output);

    return 0;
}

//using program_t = glap::model::Program<"myprogram", 
//glap::model::DefaultCommand::FirstDefined, 
//command1_t, 
//command2_t>;