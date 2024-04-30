/**
 * @file HJLogFormat.h
 * @brief This file contains macros and helper functions for logging in the application.
 *
 * The main macros provided in this file are:
 *
 * - HJ_LOG: Logs a message with a given level, event name, and format-value pairs.
 *   It uses the logger attached to the current class (if inside a class) or the
 *   "main" logger (if outside a class).
 *
 * - HJ_TLOG: Similar to HJ_LOG, but also includes the trader ID information as [tid]
 *   if the current class has an id() method, or [-] if not.
 *
 * - HJ_LOG_ID: Logs a message with a given level, event name, and format-value pairs.
 *   It includes the ID information (output as [id]) given as the 4th macro parameter.
 *   This is used when the ID cannot be obtained through the this->id() call but exists
 *   and needs to be output.
 *
 * The file also provides the VAR() macro, which is a helper macro for formatting variables
 * in the log messages.
 *
 * These macros use the Boost Preprocessor library for variadic macro processing and string
 * manipulation.
 *
 * @author Peter Liu
 * @date 20240429
 */

#pragma once

#include "LogConfig.h"
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


// Macro helper to get the format string from a tuple
#define FORMAT_STR_FROM_TUPLE(s,i,tuple) \
    BOOST_PP_IF(BOOST_PP_EQUAL(BOOST_PP_TUPLE_SIZE(tuple),2), BOOST_PP_TUPLE_ELEM(0,tuple), "{}")

// Macro helper to get the variable from a tuple
#define VAR_FROM_TUPLE(s,i,tuple) \
    BOOST_PP_IF(BOOST_PP_EQUAL(BOOST_PP_TUPLE_SIZE(tuple),2), BOOST_PP_TUPLE_ELEM(1,tuple), BOOST_PP_TUPLE_ELEM(0,tuple))

// Macro helper to juxtapose two strings with a comma separator
#define JUXTAPOSE(s, state, x) BOOST_PP_IF(BOOST_PP_EQUAL(s,1), x, state "," x)

// Macro helper to collect format strings from a variadic sequence
#define COLLECT_FORMAT_STR(...) \
      BOOST_PP_SEQ_FOLD_LEFT(JUXTAPOSE, 0,BOOST_PP_SEQ_TRANSFORM(FORMAT_STR_FROM_TUPLE, 0, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)))

// Macro helper to collect variables from a variadic sequence
#define COLLECT_VAR(...) \
      , BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(VAR_FROM_TUPLE, 0, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)))

// Helper macros for HJ_LOG and HJ_TLOG
// HJ_LOG | HJ_TLOG is used inside a class with a logger attached (via AttachLogger<"logname">).
// For classes involving Trader (e.g., Trader, TraderCallback, AlgoApi, AlgoBase, and specific algo classes)
// use HJ_TLOG as that will print out Trader Id automatically (or - if there's no id).
// Format of HJ_LOG:
//   [function name] [event name] [trader id] {information}
// Format of HJ_TLOG:
//   [function name] [event name] {information}
#define HJ_THIS(is_method)   BOOST_PP_IF(is_method, this->, wcc::)
#define HJ_HAS_ID(is_method) BOOST_PP_IF(is_method, wcc::internal::has_id<decltype(this)>::value, false)
#define HJ_ID(is_method)     BOOST_PP_IF(is_method, wcc::internal::id(this), '-') // the '-' is used when there is no id

/**
 * HJ_LOG macro for formatted output with a given level, event, and format-value pairs.
 * It uses the logger attached to the current class (is_method == 1) or the "main" logger (is_method == 0).
 */
#define HJ_LOG(is_method, level, event, ...) {                                                            \
    constexpr auto fun_name = ::wcc::get_source_function_name(std::source_location::current());           \
    HJ_THIS(is_method)log_##level("[{:16.16s}] [{:12.12s}] {{"                                            \
        BOOST_PP_IF(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__), COLLECT_FORMAT_STR, BOOST_PP_EMPTY)(__VA_ARGS__) \
        "}}", fun_name, event                                                                             \
        BOOST_PP_IF(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__), COLLECT_VAR, BOOST_PP_EMPTY)(__VA_ARGS__));      \
}

/**
 * HJ_TLOG macro for formatted output with a given level, event, and format-value pairs.
 * It uses the logger attached to the current class (is_method == 1) or the "main" logger (is_method == 0).
 * It also includes trader id information as [tid] if the current class has an id() method, or [-] if not.
 * Note that is_method == 0 means no id() is available (since there's no class/object).
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
 * HJ_LOG_ID macro for formatted output with a given level, event, and format-value pairs.
 * It uses the logger attached to the current class (is_method == 1) or the "main" logger (is_method == 0).
 * It includes the id information (output as [id]) given as the 4th macro parameter.
 *
 * Used in cases where the id cannot be obtained through the this->id() call but does exist and needs to be output.
 */
#define HJ_LOG_ID(is_method, level, event, id, ...) {                                                     \
    constexpr auto fun_name = ::wcc::get_source_function_name(std::source_location::current());           \
    HJ_THIS(is_method)log_##level("[{:16.16s}] [{:12.12s}] [{}] {{"                                       \
        BOOST_PP_IF(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__), COLLECT_FORMAT_STR, BOOST_PP_EMPTY)(__VA_ARGS__) \
        "}}", fun_name, event, id                                                                         \
        BOOST_PP_IF(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__), COLLECT_VAR, BOOST_PP_EMPTY)(__VA_ARGS__));      \
}

// Macro helpers for VAR() macro
#define VAR_1(a)      (BOOST_PP_STRINGIZE(a)":{}", a)
#define VAR_2(fmt, a) (BOOST_PP_STRINGIZE(a)":{:" fmt "}", a)
#define VAR(...)      BOOST_PP_IF(BOOST_PP_EQUAL(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__),1), VAR_1, VAR_2)(__VA_ARGS__)


// export Log macros for users
#define MLOG(...)     HJ_LOG(1, __VA_ARGS__)
#define LOG(...)      HJ_LOG(0, __VA_ARGS__)
#define ID_MLOG(...)  HJ_TLOG(1, __VA_ARGS__)
#define ID_LOG(...)   HJ_TLOG(0, __VA_ARGS__)
