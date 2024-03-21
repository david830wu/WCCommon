#pragma once

#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <string_view>
#include <unistd.h>  // for getpid()
#include <vector>
#include <charconv> // from_chars
#include <fmt/format.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>
#include <boost/container/flat_map.hpp>

namespace wcc {

struct impl {  // internal stuff; make impl a struct so that static members can be inlined and linked as one unit
    struct string_less : std::less<std::string> {
        using is_transparent = void;
        bool operator()(std::string_view sv1, std::string_view sv2) const { return sv1 < sv2; }
    };

    inline static std::string s_logger_config_file;
    inline static boost::container::flat_map<std::string, spdlog::logger, string_less> s_logger_table;
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

// Conver string like "1s", "1ms", "1us", "1ns" to duration
inline std::chrono::system_clock::duration dur_from_chars(std::string_view dur_str) {
    int dur_int = 0;
    auto [p, ec] = std::from_chars(dur_str.data(), dur_str.data() + dur_str.size(), dur_int);
    if (ec != std::errc()) throw std::runtime_error(fmt::format("invalid duration number format,{}", dur_str));
    if (dur_str.ends_with("ms")) return std::chrono::milliseconds(dur_int);
    if (dur_str.ends_with("us")) return std::chrono::microseconds(dur_int);
    if (dur_str.ends_with("ns")) return std::chrono::nanoseconds(dur_int);
    if (dur_str.ends_with("s"))  return std::chrono::seconds(dur_int);
    throw std::runtime_error(fmt::format("invalid duration unit,{}", dur_str));
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

    impl::s_logger_table.reserve(cfg["loggers"].size());
    for (auto const &name : cfg["loggers"]) {
        std::string name_str = name.as<std::string>();
        impl::s_logger_table.emplace(name_str, spdlog::logger{name_str, std::begin(sinks), std::end(sinks)});
    }

    auto format_str = cfg["default_format"].as<std::string>();
    auto lvl_str = cfg["default_level"].as<std::string>();
    auto lvl = spdlog::level::from_str(lvl_str);
    if (lvl == spdlog::level::off)
 	throw std::runtime_error("unknown log level");
    for (auto [_, logger] : impl::s_logger_table) {
        logger.set_pattern(format_str);
        logger.set_level(lvl);
    }

    // special setting
    // Disable List
    auto const& cfg_err_loggers = cfg["set_error_loggers"];
    if (!cfg_err_loggers.IsNull()) {
        auto set_error_loggers = cfg_err_loggers.as<std::vector<std::string>>();
        for (const auto &logger_name : set_error_loggers) {
            auto& logger = impl::s_logger_table.at(logger_name);
            logger.set_level(spdlog::level::err);
        }
    }

    // Enable List
    auto const& cfg_dbg_loggers = cfg["set_debug_loggers"];
    if (!cfg_dbg_loggers.IsNull()) {
        auto set_debug_loggers = cfg_dbg_loggers.as<std::vector<std::string>>();
        for (const auto &logger_name : set_debug_loggers) {
            auto& logger = impl::s_logger_table.at(logger_name);
            logger.set_level(spdlog::level::debug);
        }
    }

    for (auto [_, logger] : impl::s_logger_table) {
       logger.flush_on(spdlog::level::warn);
    }

    once = true;
}

#if defined(TEST)
    inline void config_log() {
        impl::s_logger_config_file = "TEST";
        YAML::Node cfg = YAML::Load(R"(
          default_format    : "[%m-%d %H:%M:%S.%f] [%-8l] [%-12n] %v"
          default_level     : "info"
          default_log_dir   : "./log"
          default_log_prefix: "TEST"
          sinks:
            - stdout
          loggers:
            - main
            - TEST
          set_error_loggers:
          set_debug_loggers:
            - TEST
        )");
#else
    inline void config_log(std::string_view config_file) {
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


// Free functions using main logger

template <typename... Args>
inline void log_trace(format_string_t<Args...> fmt, Args &&...args) {
    get_logger("main")->trace(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void log_debug(format_string_t<Args...> fmt, Args &&...args) {
    get_logger("main")->debug(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void log_info(format_string_t<Args...> fmt, Args &&...args) {
    get_logger("main")->info(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void log_warn(format_string_t<Args...> fmt, Args &&...args) {
    get_logger("main")->warn(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void log_error(format_string_t<Args...> fmt, Args &&...args) {
    get_logger("main")->error(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void log_critical(format_string_t<Args...> fmt, Args &&...args) {
    get_logger("main")->critical(fmt, std::forward<Args>(args)...);
}

inline void log_flush() { get_logger("main")->flush(); }

inline void log_flush_all() {
    for (auto [_, logger] : impl::s_logger_table) {
       logger.flush();
    }
}

struct LogFlusher {
    explicit LogFlusher(std::chrono::system_clock::duration dur = std::chrono::seconds(10)) : now_{}, dur_{dur} {}

    void dur(std::chrono::system_clock::duration dur) { dur_ = dur; }

    void flush(std::chrono::system_clock::time_point now) {
        if (now - now_ > dur_) {
            now_ = now;
            wcc::log_flush_all();
        }
    }

private:
    std::chrono::system_clock::time_point now_;
    std::chrono::system_clock::duration dur_;
};

} // namespace wcc
