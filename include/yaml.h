#pragma once

#include <concepts>
#include <string_view>
#include <yaml-cpp/yaml.h>

auto load_yaml(std::string_view path) -> YAML::Node;

template <typename T>
auto yaml_value_or(const YAML::Node& node, T default_value) -> T {
    if (node.IsDefined()) {
        return node.as<std::remove_cv_t<decltype(default_value)>>();
    }
    return default_value;
}

template <typename T, typename Func>
    requires std::invocable<Func> && std::same_as<std::invoke_result_t<Func>, T>
auto yaml_value_or_else(const YAML::Node& node, Func default_value_fct) -> T {
    if (node.IsDefined()) {
        return node.as<std::remove_cv_t<decltype(default_value_fct())>>();
    }
    return default_value_fct();
}