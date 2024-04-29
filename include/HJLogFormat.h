#pragma once

#include "LogConfig.h"

#define FORMAT_STR_FROM_TUPLE(s,i,tuple) \
    BOOST_PP_IF(BOOST_PP_EQUAL(BOOST_PP_TUPLE_SIZE(tuple),2), BOOST_PP_TUPLE_ELEM(0,tuple), "{}")
#define VAR_FROM_TUPLE(s,i,tuple) \
    BOOST_PP_IF(BOOST_PP_EQUAL(BOOST_PP_TUPLE_SIZE(tuple),2), BOOST_PP_TUPLE_ELEM(1,tuple), BOOST_PP_TUPLE_ELEM(0,tuple))

#define JUXTAPOSE(s, state, x) BOOST_PP_IF(BOOST_PP_EQUAL(s,1), x, state "," x)

#define COLLECT_FORMAT_STR(...) \
      BOOST_PP_SEQ_FOLD_LEFT(JUXTAPOSE, 0,BOOST_PP_SEQ_TRANSFORM(FORMAT_STR_FROM_TUPLE, 0, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)))

#define COLLECT_VAR(...) \
      , BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(VAR_FROM_TUPLE, 0, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)))

// HJ_LOG | HJ_TLOG is used inside a class which is attached a logger (via AttachLogger<"logname">).
// For classes involving Trader (e.g., Trader, TraderCallback, AlgoApi, AlgoBase, and specific algo classes)
// use HJ_TLOG as that will print out Trader Id automatically (or - if there's no id).
// Format of HJ_LOG:
//   [function name] [event name] [trader id] {information}
// Format of HJ_TLOG:
//   [function name] [event name] {information}

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
 * Note that is_method == 0 means no id() available (since there's event no class/object).
 */
#define HJ_TLOG(is_method, level, event, ...) {                                                           \
    constexpr auto fun_name = ::wcc::get_source_function_name(std::source_location::current());           \
    if constexpr (HJ_HAS_ID(is_method)) {                                                                 \
        HJ_THIS(is_method)log_##level("[{:16.16s}] [{:12.12s}] [{}] {{"                                   \
        BOOST_PP_IF(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__), COLLECT_FORMAT_STR, BOOST_PP_EMPTY)(__VA_ARGS__) \
        "}}", fun_name, event, HJ_ID(is_method)                                                           \
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
