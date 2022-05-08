/* YAMLGetField.h
 * get field from YAML config node and replace "${today}" token to today_str variable
 *
 * Usage:
 * Given a YAML config file
 * ```yaml
 * Count: 42
 * Price: 10.43
 * SomeFile: Data/${today}/some.csv
 * ```
 * 
 * load with yaml-cpp lib
 * ```cpp
 * YAML::Node config = YAML::LoadFile(conf_file);
 * std::string today_str = "20210912";
 * int         count   ; YAML_GET_FIELD(count   , config, Count   );
 * double      price   ; YAML_GET_FIELD(price   , config, Price   );
 * std::string somefile; YAML_GET_FIELD(somefile, config, SomeFile);
 * try {
 *   std::string missing ; YAML_GET_FIELD(missing , config, Missing );
 * } catch(std::runtime_error& e) {
 *   assert(e.what() == "Cannot find node \"Missing\" in YAML");
 * }
 * 
 * assert(count == 42);
 * assert(std::fabs(price - 10.42) < EPS);
 * assert(somefile == "Data/20210912/some.csv");
 * ```
 * 
 * Author: Wentao Wu
 */

#pragma once

#include <fmt/format.h>
#include <regex>
#include <yaml-cpp/yaml.h>

template<typename T>
inline auto render_config_field(T const& value, std::string const& today_str) {
    return value;
}
template<>
inline auto render_config_field<std::string>(std::string const& value, std::string const& today_str) {
    std::regex today_regex("\\$\\{today\\}");
    return std::regex_replace(value, today_regex, today_str); 
}

// variable today_str must be defined before call YAML_GET_FIELD
#define YAML_GET_FIELD(var, node, key) \
    if(YAML::Node key_field = node[#key]) { \
        using value_t = std::remove_reference_t<decltype(var)>; \
        try {  \
            var = key_field.as<value_t>(); \
            var = render_config_field(var, today_str); \
        } catch(YAML::TypedBadConversion<value_t>& e) { \
            throw std::runtime_error(fmt::format("Bad conversion of \"{}\" in Node \"{}\"", #key, #node)); \
        } \
    } else { \
        throw std::runtime_error(fmt::format("Cannot find \"{}\" in Node \"{}\"", #key, #node)); \
    }

#define YAML_GET_FIELD_NO_RENDER(var, node, key) \
    if(YAML::Node key_field = node[#key]) { \
        using value_t = std::remove_reference_t<decltype(var)>; \
        try {  \
            var = key_field.as<value_t>(); \
        } catch(YAML::TypedBadConversion<value_t>& e) { \
            throw std::runtime_error(fmt::format("Bad conversion of \"{}\" in Node \"{}\"", #key, #node)); \
        } \
    } else { \
        throw std::runtime_error(fmt::format("Cannot find \"{}\" in Node \"{}\"", #key, #node)); \
    }
