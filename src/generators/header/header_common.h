#include <string_view>
#include <string>
#include <yaml-cpp/yaml.h>
#include <utils.h>

struct Names {
    std::string name;
    std::string glapnames;
};

auto check_name(std::string_view name) -> Result<int>;
auto get_shortname(YAML::Node shortname) -> Result<std::string>;
auto get_names(YAML::Node name, YAML::Node config) -> Result<Names>;
auto normalize_string(std::string_view name) -> std::string;