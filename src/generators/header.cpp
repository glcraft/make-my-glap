
#include <glap/config.h>
#include <tl/expected.hpp>
#include <string_view>
#include <fmt/format.h>
#include <vector>
#include <fstream>
#include <concepts>
#include <yaml.h>
#include <utils.h>

auto header_program(const YAML::Node& config, std::ofstream& output) -> Result<int>;

auto generate_header(YAML::Node config, std::ofstream output) -> Result<int> {
    auto program_config = config["program"];
    if (!program_config.IsDefined() || program_config.IsNull()) {
        return tl::make_unexpected(std::string{"No program defined"});
    }
    output << "#pragma once\n\n";
#ifdef GLAP_USE_MODULE
    output << "import glap;\n\n";
#else 
    output << "#include <glap/glap.h>\n";
#endif
    if (auto includes = program_config["includes"]; includes.IsDefined() && !includes.IsNull() && includes.IsSequence()) {
        for (auto include : includes) {
            output << "#include " << include.as<std::string>() << '\n';
        }
    }
    output << '\n';
    return header_program(program_config, output);
}