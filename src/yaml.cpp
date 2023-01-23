#include <yaml.h>
#include <utils.h>

auto load_yaml(std::string_view path) -> YAML::Node {
    auto yaml_path = std::string{path};
    try {
        return YAML::LoadFile(yaml_path);
    } catch (YAML::BadFile &e) {
        emit_error(fmt::format("failed to open file '{}'", yaml_path));
    } catch (YAML::ParserException &e) {
        emit_error(fmt::format("failed to parse file '{}'\n{}", yaml_path, e.what()));
    }
    return {};
}

