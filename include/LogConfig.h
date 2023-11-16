#pragma once

#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <unistd.h> // for getpid()
#include <unordered_map>
#include <vector>
#include <string_view>

#include <fmt/format.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>

namespace wcc {

struct impl {  // internal stuff; make impl a struct so that static members can be inlined and linked as one unit
    struct string_hash {
        using hash_type = std::hash<std::string_view>;
        using is_transparent = void;

        std::size_t operator()(const char *str) const { return hash_type{}(str); }
        std::size_t operator()(std::string_view str) const {
            return hash_type{}(str);
        }
        std::size_t operator()(std::string const &str) const {
            return hash_type{}(str);
        }
    };

    inline static std::string s_logger_config_file;
    inline static std::unordered_map<std::string, spdlog::logger, string_hash, std::equal_to<>> s_logger_table;
};  // struct impl

inline spdlog::logger*
get_logger(std::string_view logger_name) {
    auto iter = impl::s_logger_table.find(logger_name);
    if (iter != impl::s_logger_table.end()) [[likely]]
        return &(iter->second);

    fmt::print(stderr, "get_logger,MissingLogger,logger={},LogConfig={}\n",
               logger_name, impl::s_logger_config_file);
    throw std::runtime_error(fmt::format("MissingLogger:{}", logger_name));
}

inline void config_log(YAML::Node const& cfg) {
    static bool once = false;

    if (once) {
        std::cerr << "loggers has been created before from "
                  << impl::s_logger_config_file << std::endl;
        return;
    }

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream today_oss;
    today_oss << std::put_time(&tm, "%Y%m%d");
    std::string today_str = today_oss.str();

    std::regex today_regex("\\$\\{today\\}");
    std::string default_log_dir = cfg["default_log_dir"].as<std::string>();
    default_log_dir =
        std::regex_replace(default_log_dir, today_regex, today_str);
    std::string default_log_prefix =
        cfg["default_log_prefix"].as<std::string>();
    default_log_prefix =
        std::regex_replace(default_log_prefix, today_regex, today_str);

    std::string log_file = default_log_dir + '/' + default_log_prefix + ".pid." + std::to_string(getpid()) + ".log";

    // create log_dir if not exist
    std::filesystem::create_directories(default_log_dir);

    std::vector<spdlog::sink_ptr> sinks;
    auto sink_table = cfg["sinks"].as<std::vector<std::string>>();
    for (const auto &sink : sink_table) {
        if (sink == "stdout")
            sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());
        if (sink == "basic_file")
            sinks.push_back(
                std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file));
    }

    for (auto const &name : cfg["loggers"]) {
        std::string name_str = name.as<std::string>();
        impl::s_logger_table.emplace(name_str, spdlog::logger{name_str, std::begin(sinks), std::end(sinks)});
    }

    auto format_str = cfg["default_format"].as<std::string>();
    auto lvl_str = cfg["default_level"].as<std::string>();
    auto lvl = lvl_str == "info"   ? spdlog::level::info
               : lvl_str == "warn" ? spdlog::level::warn
               : lvl_str == "err"  ? spdlog::level::err
	       : lvl_str == "debug"? spdlog::level::debug
               : lvl_str == "critical" ? spdlog::level::critical
               : throw std::runtime_error("unknown log level");
    for (auto [_, logger] : impl::s_logger_table) {
        logger.set_pattern(format_str);
        logger.set_level(lvl);
    }

    // special setting
    // Disable List
    auto set_error_loggers = cfg["set_error_loggers"].as<std::vector<std::string>>();
    for (const auto &logger_name : set_error_loggers) {
        auto& logger = impl::s_logger_table.at(logger_name);
        logger.set_level(spdlog::level::err);
    }

    // Enable List
    auto set_debug_loggers = cfg["set_debug_loggers"].as<std::vector<std::string>>();
    for (const auto &logger_name : set_debug_loggers) {
        auto& logger = impl::s_logger_table.at(logger_name);
        logger.set_level(spdlog::level::debug);
    }

    for (auto [_, logger] : impl::s_logger_table) logger.flush_on(spdlog::level::warn);

    once = true;
}

inline void config_log(std::string_view config_file) {
    #if defined(TEST)
        impl::s_logger_config_file = "TEST";
        YAML::Node config = YAML::Load(R"(
          default_format    : "[%m-%d %H:%M:%S.%f] [%-8l] [%-12n] %v"
          default_level     : "info"
          default_log_dir   : "./log"
          default_log_prefix: "TEST"
          sinks:
            - stdout
          loggers:
            - default
            - TEST
          set_error_loggers:
            - default 
          set_debug_loggers:
            - TEST        
        )");
    #else
        impl::s_logger_config_file = config_file;
        YAML::Node cfg = YAML::LoadFile(impl::s_logger_config_file);
    #endif
    config_log(cfg);
}

// Following need c++20
template <size_t N> struct StringLiteral {
    constexpr StringLiteral(const char (&str)[N]) {
        std::copy_n(str, N, value);
    }
    char value[N];
};

template <typename... Args>
using format_string_t = spdlog::format_string_t<Args...>;

template <StringLiteral STR> class AttachLogger {
public:
#if defined(TEST)
    AttachLogger() : p_logger_(get_logger("TEST")) {}
#else
    AttachLogger() : p_logger_(get_logger(STR.value)) {}
#endif

    template <typename... Args>
    void log_trace(format_string_t<Args...> fmt, Args &&...args) const {
        p_logger_->trace(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void log_debug(format_string_t<Args...> fmt, Args &&...args) const {
        p_logger_->debug(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void log_info(format_string_t<Args...> fmt, Args &&...args) const {
        p_logger_->info(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void log_warn(format_string_t<Args...> fmt, Args &&...args) const {
        p_logger_->warn(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void log_error(format_string_t<Args...> fmt, Args &&...args) const {
        p_logger_->error(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void log_critical(format_string_t<Args...> fmt, Args &&...args) const {
        p_logger_->critical(fmt, std::forward<Args>(args)...);
    }

    void log_flush() const { p_logger_->flush(); }

protected:
    mutable spdlog::logger* p_logger_;
};


// Free functions using default logger

template <typename... Args>
inline void log_trace(format_string_t<Args...> fmt, Args &&...args) {
    get_logger("default")->trace(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void log_debug(format_string_t<Args...> fmt, Args &&...args) {
    get_logger("default")->debug(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void log_info(format_string_t<Args...> fmt, Args &&...args) {
    get_logger("default")->info(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void log_warn(format_string_t<Args...> fmt, Args &&...args) {
    get_logger("default")->warn(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void log_error(format_string_t<Args...> fmt, Args &&...args) {
    get_logger("default")->error(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void log_critical(format_string_t<Args...> fmt, Args &&...args) {
    get_logger("default")->critical(fmt, std::forward<Args>(args)...);
}

inline void log_flush() { get_logger("default")->flush(); }


} // namespace wcc
