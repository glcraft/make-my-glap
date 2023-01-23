#include <glap_def.h>
#include <string_view>
#include <yaml-cpp/yaml.h>
#include <span>
#include <iostream>
#include <fstream>
#include <ranges>
#include <fmt/format.h>

#include <generators.h>
#include <utils.h>
#include <yaml.h>


int main(int argc, char** argv) {
    std::vector<std::string_view> args{argv, argv+argc};
    auto args_result = glap::parser<args::program_t>(args);
    if (!args_result) {
        std::cerr << args_result.error().to_string() << std::endl;
        return 1;
    }
    auto program = args_result.value();
    auto &command = std::get<args::command_generate_t>(program.command);
    if (!command.get_argument<"input">().value) {
        emit_error("input path is not set");
    }
    auto yaml_path = std::string{command.get_argument<"input">().value.value()};
    auto config = load_yaml(yaml_path);

    auto type = command.get_argument<"output-type">().value.value_or("header");
    auto output_path = std::string{command.get_argument<"output">().value.value_or("output.h")};
    auto output = std::ofstream(output_path.c_str());
    
    if (!output) {
        emit_error(fmt::format("Failed to open file for writing '{}'", output_path));
    }
    auto result = [&]() -> Result<int> {
        if (type == "header") {
            return generate_header(std::move(config), std::move(output));
        }
        return tl::make_unexpected(fmt::format("Unknown type '{}'", type));
    }();
    
    if (!result) {
        emit_error(result.error());
    }
    
    return 0;
}