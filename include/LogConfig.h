#pragma once

#include <unistd.h>  // for getpid()
#include <ctime>
#include <filesystem>
#include <regex>
#include <sstream>
#include <string_view>
#include <vector>
#include <charconv> // from_chars
#include <source_location>
#include <fmt/format.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>
#include <boost/container/flat_map.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/variadic.hpp>
#include <boost/preprocessor/seq/seq.hpp>
#include <boost/preprocessor/seq/transform.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/seq/fold_left.hpp>
#include <boost/preprocessor/facilities/empty.hpp>
#include <boost/preprocessor/comparison/equal.hpp>

namespace wcc {

namespace internal {

struct impl {  // make impl a struct so that static members can be inlined and linked as one unit
    struct string_less : std::less<std::string> {
        using is_transparent = void;
        bool operator()(std::string_view sv1, std::string_view sv2) const { return sv1 < sv2; }
    };

    inline static std::string s_logger_config_file;
    inline static boost::container::flat_map<std::string, spdlog::logger, string_less> s_loggers;
    inline static std::ostringstream s_oss;
};  // struct impl

// T must be a pointer (since it is used as has_id<this> for now)
template <typename T, typename = void> struct has_id : std::false_type {};
template <typename T> struct has_id<T, std::void_t<decltype((*std::declval<T>()).id())>> : std::true_type {};
template <typename T>
auto id(const T* t) {
    if constexpr (has_id<const T*>::value) return t->id();
    else return 0; // doesn't matter what returns (both type and value) here since it will be discarded by compiler
}

} // namespace internal


inline spdlog::logger* get_logger(std::string_view logger_name) {
    auto iter = internal::impl::s_loggers.find(logger_name);
    if (iter != internal::impl::s_loggers.end()) [[likely]]
        return &(iter->second);

    fmt::print(stderr, "get_logger,MissingLogger,logger={},LogConfig={}\n",
               logger_name, internal::impl::s_logger_config_file);
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

// Note that every get_logger_str call will clear the underlying string
inline auto get_logger_str() {
    auto str_view = internal::impl::s_oss.view();
    internal::impl::s_oss.seekp(0);
    return str_view;
}

inline void config_log(YAML::Node const& cfg) {
    static bool once = false;

    if (once) {
        fmt::print("loggers has been created before from {}\n", internal::impl::s_logger_config_file);
        return;
    }

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream today_oss;
    today_oss << std::put_time(&tm, "%Y%m%d");
    std::string today_str = today_oss.str();

    std::regex today_regex("\\$\\{today\\}");

    std::vector<spdlog::sink_ptr> sinks;
    auto sink_table = cfg["sinks"].as<std::vector<std::string>>();
    for (const auto &sink : sink_table) {
        if (sink == "stdout")
            sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_mt>());
        if (sink == "basic_file") {
            std::string default_log_dir = cfg["default_log_dir"].as<std::string>();
            default_log_dir = std::regex_replace(default_log_dir, today_regex, today_str);
            std::string default_log_prefix = cfg["default_log_prefix"].as<std::string>();
            default_log_prefix = std::regex_replace(default_log_prefix, today_regex, today_str);
            std::string log_file = default_log_dir + '/' + default_log_prefix + ".pid." + std::to_string(getpid()) + ".log";
            std::filesystem::create_directories(default_log_dir);  // create log_dir if not exist
            sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file));
        }
        if (sink == "string") {
            sinks.push_back(std::make_shared<spdlog::sinks::ostream_sink_mt>(internal::impl::s_oss));
        }
    }

    internal::impl::s_loggers.reserve(cfg["loggers"].size());
    for (auto const &name : cfg["loggers"]) {
        std::string name_str = name.as<std::string>();
        internal::impl::s_loggers.emplace(name_str, spdlog::logger{name_str, std::begin(sinks), std::end(sinks)});
    }

    auto format_str = cfg["default_format"].as<std::string>();
    auto lvl_str = cfg["default_level"].as<std::string>();
    auto lvl = spdlog::level::from_str(lvl_str);
    if (lvl == spdlog::level::off)
 	throw std::runtime_error("unknown log level");
    for (auto [_, logger] : internal::impl::s_loggers) {
        logger.set_pattern(format_str);
        logger.set_level(lvl);
    }

    // special setting
    // Disable List
    auto const& cfg_err_loggers = cfg["set_error_loggers"];
    if (!cfg_err_loggers.IsNull()) {
        auto set_error_loggers = cfg_err_loggers.as<std::vector<std::string>>();
        for (const auto &logger_name : set_error_loggers) {
            auto& logger = internal::impl::s_loggers.at(logger_name);
            logger.set_level(spdlog::level::err);
        }
    }

    // Enable List
    auto const& cfg_dbg_loggers = cfg["set_debug_loggers"];
    if (!cfg_dbg_loggers.IsNull()) {
        auto set_debug_loggers = cfg_dbg_loggers.as<std::vector<std::string>>();
        for (const auto &logger_name : set_debug_loggers) {
            auto& logger = internal::impl::s_loggers.at(logger_name);
            logger.set_level(spdlog::level::debug);
        }
    }

    for (auto [_, logger] : internal::impl::s_loggers) {
       logger.flush_on(spdlog::level::warn);
    }

    once = true;
}

#if defined(TEST)
inline void config_log() {
    internal::impl::s_logger_config_file = "TEST";
    YAML::Node cfg = YAML::Load(R"(
        default_format    : "[%m-%d %H:%M:%S.%f] [%-8l] [%-12n] %v"
        default_level     : "info"
        sinks:
        - stdout
        loggers:
        - main
        set_error_loggers:
        set_debug_loggers:
    )");
#else
inline void config_log(std::string_view config_file) {
    internal::impl::s_logger_config_file = config_file;
    YAML::Node cfg = YAML::LoadFile(internal::impl::s_logger_config_file);
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
    AttachLogger() : p_logger_(get_logger("main")) {}
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
    for (auto [_, logger] : internal::impl::s_loggers) {
       logger.flush();
    }
}


/**
 * get_source_function_name: used by macros following
*/
inline constexpr auto get_source_function_name(std::source_location loc) {
    auto name = std::string_view(loc.function_name());
    auto e = name.find_first_of('(');
    auto b = name.find_last_of(':', e);  // for ::
    if (b == std::string_view::npos) b = name.find_first_of(' ');
    return std::string_view(name.begin()+b+1, name.begin()+e);
}

/**
 * is_method: check if current inside a nonstatic method or not; not used so far
*/
inline constexpr auto is_inside_method(std::source_location loc) {
    auto name = std::string_view(loc.function_name());
    // Not a pattern of static R class::foo() or R foo()
    return name.find("static ") == name.npos && name.find("::") != name.npos;
}

} // namespace wcc


#define FORMAT_STR_FROM_TUPLE(s,i,tuple) \
    BOOST_PP_IF(BOOST_PP_EQUAL(BOOST_PP_TUPLE_SIZE(tuple),2), BOOST_PP_TUPLE_ELEM(0,tuple), "{}")
#define VAR_FROM_TUPLE(s,i,tuple) \
    BOOST_PP_IF(BOOST_PP_EQUAL(BOOST_PP_TUPLE_SIZE(tuple),2), BOOST_PP_TUPLE_ELEM(1,tuple), BOOST_PP_TUPLE_ELEM(0,tuple))

#define JUXTAPOSE(s, state, x) BOOST_PP_IF(BOOST_PP_EQUAL(s,1), x, state "," x)

#define COLLECT_FORMAT_STR(...) \
      BOOST_PP_SEQ_FOLD_LEFT(JUXTAPOSE, 0,BOOST_PP_SEQ_TRANSFORM(FORMAT_STR_FROM_TUPLE, 0, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)))

#define COLLECT_VAR(...) \
      , BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(VAR_FROM_TUPLE, 0, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)))

// HJ_LOG used inside a class attcheded with a logger (via AttachLogger<"logname">).
// For classes involving Trader (e.g., Trader, TraderCallback, AlgoApi, AlgoBase, and specific algo classes)
// it will print out Trader Id automatically, otherwise, empty (a blank space).
// Format:
//   [function name] [event name] [trader id] {information}

#define HJ_THIS(is_method)   BOOST_PP_IF(is_method, this->, wcc::)
#define HJ_HAS_ID(is_method) BOOST_PP_IF(is_method, wcc::internal::has_id<decltype(this)>::value, false)
#define HJ_ID(is_method)     BOOST_PP_IF(is_method, wcc::internal::id(this), '-') // the '-' actually no use, just make grammar happy in HJ_TLOG

/**
 * HJ_LOG formatted ouput with given level, event, and format-value pairs, using logger
 * of the current class attached (is_method == 1), or the "main" logger (is_method == 0).
 */
#define HJ_LOG(is_method, level, event, ...) {                                                            \
    constexpr auto fun_name = ::wcc::get_source_function_name(std::source_location::current());           \
    HJ_THIS(is_method)log_##level("[{:16.16s}] [{:12.12s}] {{"                                            \
        BOOST_PP_IF(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__), COLLECT_FORMAT_STR, BOOST_PP_EMPTY)(__VA_ARGS__) \
        "}}", fun_name, event                                                                             \
        BOOST_PP_IF(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__), COLLECT_VAR, BOOST_PP_EMPTY)(__VA_ARGS__));      \
}

/**
 * HJ_TLOG formatted ouput with given level, event, and format-value pairs, using logger
 * of the current class attached (is_method == 1), or the "main" logger (is_method == 0),
 * plus trader id infomation as [tid] if current class has an id() method, or [-] if not.
 * Note that is_method == 0 means no id() avaiable (since there's event no class/object).
 */
#define HJ_TLOG(is_method, level, event, ...) {                                                           \
    constexpr auto fun_name = ::wcc::get_source_function_name(std::source_location::current());           \
    if constexpr (HJ_HAS_ID(is_method)) {                                                                 \
        HJ_THIS(is_method)log_##level("[{:16.16s}] [{:12.12s}] [{}] {{"                                   \
        BOOST_PP_IF(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__), COLLECT_FORMAT_STR, BOOST_PP_EMPTY)(__VA_ARGS__) \
        "}}", fun_name, event, HJ_ID(is_method)                                                            \
        BOOST_PP_IF(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__), COLLECT_VAR, BOOST_PP_EMPTY)(__VA_ARGS__));      \
    } else {                                                                                              \
        HJ_THIS(is_method)log_##level("[{:16.16s}] [{:12.12s}] [-] {{"                                    \
        BOOST_PP_IF(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__), COLLECT_FORMAT_STR, BOOST_PP_EMPTY)(__VA_ARGS__) \
        "}}", fun_name, event                                                                             \
        BOOST_PP_IF(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__), COLLECT_VAR, BOOST_PP_EMPTY)(__VA_ARGS__));      \
    }                                                                                                     \
}

/**
 * HJ_LOG_ID formatted ouput with given level, event, and format-value pairs, using logger
 * of the current class attached (is_method == 1), or the "main" logger (is_method == 0),
 * plus id infomation (output as [id]) given as 4th macro parameter.
 *
 * Used in cases that id can not be obtained as this->id() call but id does exist and need to be output.
 */
#define HJ_LOG_ID(is_method, level, event, id, ...) {                                                     \
    constexpr auto fun_name = ::wcc::get_source_function_name(std::source_location::current());           \
    HJ_THIS(is_method)log_##level("[{:16.16s}] [{:12.12s}] [{}] {{"                                       \
        BOOST_PP_IF(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__), COLLECT_FORMAT_STR, BOOST_PP_EMPTY)(__VA_ARGS__) \
        "}}", fun_name, event, id                                                                         \
        BOOST_PP_IF(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__), COLLECT_VAR, BOOST_PP_EMPTY)(__VA_ARGS__));      \
}

#define VAR_1(a)      (BOOST_PP_STRINGIZE(a)":{}", a)
#define VAR_2(fmt, a) (BOOST_PP_STRINGIZE(a)":{:" fmt "}", a)
#define VAR(...)      BOOST_PP_IF(BOOST_PP_EQUAL(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__),1), VAR_1, VAR_2)(__VA_ARGS__)

/* EXAMPLES:
HJ_LOG(1, info, event, ("{:3d}", 5), (f(x)), ("hello"), ("{:.10s}","pliu"));
HJ_LOG(1, debug, event);
HJ_LOG(0, debug, event, VAR(price), VAR(vol));  // VAR(x) expand to ("x={}", x)

HJ_TLOG(1, info, event, ("{:3d}", 5), (f(x)), ("hello"), ("{:.10s}","pliu"));
HJ_TLOG(1, debug, event);
HJ_TLOG(0, debug, event, VAR(price), VAR(vol));  // VAR(x) expand to ("x={}", x)
*/