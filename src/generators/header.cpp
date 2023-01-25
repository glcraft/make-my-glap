
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
auto header_help(const YAML::Node& config, std::ofstream& output) -> Result<std::string>;

auto header_includes(const YAML::Node& program_config, std::ofstream& output) {
    if (auto includes = program_config["includes"]; includes.IsDefined() && !includes.IsNull() && includes.IsSequence()) {
        for (auto include : includes) {
            output << "#include " << include.as<std::string>() << '\n';
        }
    }
    output << '\n';
}

auto header_modules(const YAML::Node& program_config, std::ofstream& output) {
    if (auto modules = program_config["modules"]; modules.IsDefined() && !modules.IsNull() && modules.IsSequence()) {
        for (auto module : modules) {
            output << "import " << module.as<std::string>() << ";\n";
        }
    }
    output << '\n';
}

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
    output << "#include <glap/generators/help.h>\n";
#endif
    
    header_includes(program_config, output);
    header_modules(program_config, output);

    if (auto result = header_program(program_config, output); !result) {
        return tl::make_unexpected(result.error());
    }
    if (auto result = header_help(program_config, output); !result) {
        return tl::make_unexpected(result.error());
    }
    return 0;
}