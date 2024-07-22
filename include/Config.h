#pragma once

#include <yaml-cpp/yaml.h>
#include <fmt/format.h>
#include <regex>
#include <filesystem>
#include <ranges>

namespace wcc {

using Config = YAML::Node;

template <typename T = std::string>
T get_field(Config const& cfg, std::string_view key) {
    try {
        return cfg[std::string(key)].template as<T>();
    } catch(std::exception const& e) {
        throw std::runtime_error(fmt::format("Exception for config key \"{}\": {}", key, e.what()));
    }
}

using Replacements = std::vector<std::pair<std::string, std::string>>;

YAML::Node load_full_config(std::string const& cfg_file, Replacements const& replacements,
                            std::string cfg_root = {}, std::filesystem::path path = {});

static inline void load_full_config(YAML::Node& node, Replacements const& replacements,
                                    std::string cfg_root, std::filesystem::path path) {
    if (node.IsMap()) {  // root node must be a Map
        for (auto it = node.begin(); it != node.end(); ++it) {
            load_full_config(it->second, replacements, cfg_root, path);
        }
    } else if (node.IsSequence()) {
    } else if (node.IsScalar()) {
        auto str = node.as<std::string>();
        if (str.ends_with(".yaml")) {  // && !std::regex_search(str, std::regex("\\$\\{.*\\}"))) {
            auto n = load_full_config(str, replacements, cfg_root, path);
            node = n;
        }
        else {  // do the rendering
            for (auto& [k, v] : replacements) {
                str = std::regex_replace(str, std::regex(std::string("\\$\\{") + k + "\\}"), v);
            }
            node = str;
        }
    } // else other type to process; to be added in the future
}

inline YAML::Node load_full_config(std::string const& cfg_file, Replacements const& replacements,
                                   std::string cfg_root, std::filesystem::path path)
{
    namespace fs = std::filesystem;

    if (cfg_root.empty()) cfg_root = fs::absolute(cfg_file).parent_path().string();

    auto cfg_fname = std::regex_replace(cfg_file, std::regex(std::string("\\$\\{cfg_root\\}")), cfg_root);

    fs::path cfg_path(cfg_fname);
    // cfg_name == cfg_file means no cfg_root specified so take relative into account
    if (cfg_fname == cfg_file && cfg_path.is_relative()) {
        cfg_path = path / cfg_path;
    }

    path = cfg_path.parent_path();

    if (!std::filesystem::exists(cfg_path))
        throw std::runtime_error(fmt::format("file {} does not exist", cfg_path.c_str()));

    fmt::print("Loading {}\n", cfg_path.native());
    auto node = YAML::LoadFile(cfg_path.native());
    load_full_config(node, replacements, cfg_root, path);
    return node;
}

}  // namespace wcc

/**
 * Example: g++ -std=c++20 -I ~/dev/include -lyaml-cpp -g -o cfg cfg.cpp
 */
// int main() {
//     auto cfg = load_full_config("Config/ManualWCTraderConfig.yaml", std::array{std::pair{"today", "20230927"}});
//     fmt::print("{}\n", cfg);
// }
