#pragma once

#include <string_view>
#include <yaml-cpp/yaml.h>

auto load_yaml(std::string_view path) -> YAML::Node;

auto yaml_value_or(const YAML::Node& node, auto default_value) -> decltype(default_value);
auto yaml_value_or_else(const YAML::Node& node, auto default_value_fct) -> decltype(default_value_fct());