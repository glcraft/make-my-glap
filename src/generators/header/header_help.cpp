#include <yaml-cpp/yaml.h>
#include <fmt/format.h>
#include <yaml.h>
#include <utils.h>
#include "header_common.h"

auto header_help(const YAML::Node& program, std::ofstream& output) -> Result<std::string> {
    const auto help_typename = "help_t";
    auto help_description = program["description"];
    if (!help_description.IsDefined()) {
        return tl::make_unexpected(fmt::format("program: Missing description"));
    }
    auto help_description_str = help_description.as<std::string>();
    // if ()
    auto program_name = program["name"].as<std::string>();

    output 
        << fmt::format("using {} = glap::model::Help<{}, {}>;\n", help_typename, program_name, help_description);
    return help_typename;
}