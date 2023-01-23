
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
    output << "#pragma once\n\n";
#ifdef GLAP_USE_MODULE
    output << "import glap;\n\n";
#else 
    output << "#include <glap/glap.h>\n\n";
#endif
    return header_program(config["program"], output);
}