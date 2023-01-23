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

auto yaml_value_or(const YAML::Node& node, auto default_value) -> decltype(default_value) {
    if (node.IsDefined()) {
        return node.as<std::remove_cv_t<decltype(default_value)>>();
    }
    return default_value;
}
auto yaml_value_or_else(const YAML::Node& node, auto default_value_fct) -> decltype(default_value_fct()) {
    if (node.IsDefined()) {
        return node.as<std::remove_cv_t<decltype(default_value_fct())>>();
    }
    return default_value_fct();
}