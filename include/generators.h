#pragma once

#include <yaml-cpp/yaml.h>
#include <fstream>
#include <tl/expected.hpp>
#include <utils.h>

auto generate_header(YAML::Node config, std::ofstream output) -> Result;