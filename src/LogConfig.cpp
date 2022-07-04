/* LogConfig.cpp
 * Implement config_log function
 *
 * Author: Wentao Wu
*/

#include "LogConfig.h"
#include "WCCommonUtils.h"

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <regex>
#include <unistd.h> // for getpid()
#include <unordered_map>
#include <vector>

#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <yaml-cpp/yaml.h>


namespace wcc {

namespace impl {
    static std::string s_logger_config_file;
    static std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> s_logger_table;
    static const std::unordered_map<std::string, spdlog::level::level_enum> s_logger_level = {
        {"debug"   , spdlog::level::debug   },
        {"info"    , spdlog::level::info    },
        {"warn"    , spdlog::level::warn    },
        {"err"     , spdlog::level::err     },
        {"critical", spdlog::level::critical},
    };
}

std::shared_ptr<spdlog::logger> get_logger(std::string_view logger_name) {
    auto iter = impl::s_logger_table.find(static_cast<std::string>(logger_name));
    if(iter == impl::s_logger_table.end()) {
        std::cerr << fmt::format("get_logger,MissingLogger,logger={},LogConfig={}", logger_name, impl::s_logger_config_file) << std::endl;
        throw std::runtime_error(fmt::format("MissingLogger:{}", logger_name));
    }
    return iter->second;
}

void config_log(std::string_view config_file) {
    static bool once = false;
    if(!once) {
        impl::s_logger_config_file = static_cast<std::string>(config_file);
        YAML::Node config = YAML::LoadFile(impl::s_logger_config_file);

        std::regex today_regex("\\$\\{today\\}");
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::ostringstream today_oss;
        today_oss << std::put_time(&tm, "%Y%m%d");
        std::string today_str = today_oss.str();

        std::string log_file("");

        std::string default_log_dir = config["default_log_dir"].as<std::string>();
        default_log_dir = std::regex_replace(default_log_dir, today_regex, today_str);
        log_file += default_log_dir;
        log_file += "/";
        std::string default_log_prefix = config["default_log_prefix"].as<std::string>();
        default_log_prefix = std::regex_replace(default_log_prefix, today_regex, today_str);
        log_file += default_log_prefix;
        log_file += ".pid.";
        log_file += std::to_string(getpid());
        log_file += ".log";

        // create log_dir if not exist
        mkdir_if_not_exist(default_log_dir);

        std::vector<spdlog::sink_ptr> sinks;
        auto sink_table = config["sinks"].as< std::vector<std::string> >();
        for(const auto& sink : sink_table) {
            if (sink == "stdout")
                sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());
            if (sink == "basic_file")
                sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file));
        }

        for(auto name: config["loggers"]) {
            std::string name_str = name.as<std::string>();
            impl::s_logger_table[name_str] = std::make_shared<spdlog::logger>(name_str, std::begin(sinks), std::end(sinks));
        }

        std::string format_str = config["default_format"].as<std::string>();
        for(auto iter: impl::s_logger_table) {
            iter.second->set_pattern(format_str);
            iter.second->set_level(impl::s_logger_level.at(config["default_level"].as<std::string>()));
        }

        // special setting
        // Disable List
        auto set_error_loggers = config["set_error_loggers"].as< std::vector<std::string> >();
        for(const auto& lg : set_error_loggers) {
            std::string logger_name = lg;
            auto p_logger = impl::s_logger_table.at(logger_name);
            p_logger->set_level(impl::s_logger_level.at("err"));
        }

        // Enable List
        auto set_debug_loggers = config["set_debug_loggers"].as< std::vector<std::string> >();
        for(const auto& lg : set_debug_loggers) {
            std::string logger_name = lg;
            auto p_logger = impl::s_logger_table.at(logger_name);
            p_logger->set_level(impl::s_logger_level.at("debug"));
        }

        // register all loggers
        for(auto iter: impl::s_logger_table) {
            spdlog::register_logger(iter.second);
        }

        once = true;
    } else {
        std::cerr << "loggers has been created before from " << impl::s_logger_config_file << std::endl;
    }

}

} /* namespace wcc */
