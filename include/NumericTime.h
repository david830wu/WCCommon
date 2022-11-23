/* NumericTime.h
 * 
 * Author: Wentao Wu
*/

#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <chrono>
#include <limits>
#include <date/date.h>

#include <time.h>

namespace wcc {

    // class NumericTime 
    // NumericTime stores time as unsigned int with millisecond resolution
    // 
    class NumericTime {
    public:
        static const NumericTime NaN;

        constexpr NumericTime(unsigned int int_time = 240000'000)
          : data_(int_time)
        {}

        NumericTime(const std::string& value) {
            int hour = 24, min = 0, sec = 0, ms = 0;
            std::sscanf(value.c_str(), "%02d:%02d:%02d.%03d", &hour, &min, &sec, &ms);
            data_ = to_timestamp(hour, min, sec, ms);
        }
        NumericTime(int hour, int min, int sec, int ms = 0)
          : data_(to_timestamp(hour, min, sec, ms))
        {}
        ~NumericTime() = default;

        operator unsigned int() const { return data_; }

        std::string str() const { 
            constexpr int buffer_size = 16;
            char buffer[buffer_size];
            std::memset(buffer, 0, buffer_size);
            if(data_ >= 240000'000) {
                std::strncpy(buffer, "NaN", buffer_size);
            } else {
                std::snprintf(buffer, buffer_size, "%02d:%02d:%02d.%03d", hour(), min(), sec(), ms());
            }
            return buffer; 
        }
        static NumericTime now() {
            auto tp = std::chrono::system_clock::now();
            auto dp = date::floor<date::days>(tp);
            auto ymd = date::year_month_day{dp};
            auto time = date::make_time(std::chrono::duration_cast<std::chrono::milliseconds>(tp-dp));
            return NumericTime(
                time.hours().count() + 8, // to beijing time
                time.minutes().count(),
                time.seconds().count(),
                time.subseconds().count()
            );
        }
        unsigned int hour() const noexcept {
            int remains = data_, hour;
            /*ms   = remains % 1000;*/ remains /= 1000;
            /*sec  = remains % 100 ;*/ remains /= 100 ;
            /*min  = remains % 100 ;*/ remains /= 100 ;
            hour = remains % 100 ; 
            return hour;
        }
        unsigned int min() const noexcept {
            int remains = data_, min;
            /*ms   = remains % 1000;*/ remains /= 1000;
            /*sec  = remains % 100 ;*/ remains /= 100 ;
            min  = remains % 100 ;
            return min;
        }
        unsigned int sec() const noexcept {
            int remains = data_, sec;
            /*ms   = remains % 1000;*/ remains /= 1000;
            sec  = remains % 100 ; 
            return sec;
        }
        unsigned int ms() const noexcept {
            int remains = data_, ms;
            ms   = remains % 1000; 
            return ms;
        }
        unsigned int total_ms() const noexcept {
            return (((hour() * 60) + min()) * 60 + sec()) * 1000 + ms();
        }
        unsigned int data() const noexcept {
            return data_;
        }

        friend std::ostream& operator<<(std::ostream& os, const NumericTime& value) {
            os << value.str();
            return os;
        }

    private:

        unsigned int to_timestamp(int hour, int min, int sec, int ms) {
            unsigned int timestamp = 240000'000;
            while(ms   < 0) { --sec ; ms  += 1000; }
            while(sec  < 0) { --min ; sec += 60  ; }
            while(min  < 0) { --hour; min += 60  ; }
            while(hour < 0) { hour += 24; }
            sec  += ms  / 1000; ms  = ms  % 1000;
            min  += sec / 60  ; sec = sec % 60  ;
            hour += min / 60  ; min = min % 60  ;
            hour = hour % 24;
            timestamp = hour;
            timestamp *= 100 ; timestamp += min;
            timestamp *= 100 ; timestamp += sec;
            timestamp *= 1000; timestamp += ms ;
            return timestamp;
        }

    private:

        unsigned int data_;

    };  // class NumericTime

    inline constexpr NumericTime NumericTime::NaN = 240000'000; // requires c++17

} // namespace wcdb

namespace std {
    template<> class numeric_limits<wcc::NumericTime> {
public:
    static constexpr bool is_specialized = true;
 
    static constexpr wcc::NumericTime min() noexcept { return wcc::NumericTime(0); }
    static constexpr wcc::NumericTime max() noexcept { return wcc::NumericTime(235959'999); }
    static constexpr wcc::NumericTime lowest() noexcept { return wcc::NumericTime(0); }
 
    static constexpr bool is_signed = false;
    static constexpr bool is_integer = true;
    static constexpr bool is_exact = true;
 
    static constexpr bool has_infinity = true;
    static constexpr bool has_quiet_NaN = true;
    static constexpr bool has_signaling_NaN = false;
    static constexpr wcc::NumericTime infinity() noexcept { return wcc::NumericTime::NaN; }
    static constexpr wcc::NumericTime quiet_NaN() noexcept { return wcc::NumericTime::NaN; }
 
    static constexpr bool is_bounded = true;
    static constexpr bool is_modulo = false;
    };

    inline std::string to_string(wcc::NumericTime const& ntime) {
        constexpr int k_max_buffer_len = 12;
        char buffer[k_max_buffer_len];
        snprintf(buffer, k_max_buffer_len, "%09u", static_cast<unsigned int>(ntime));
        return buffer;
    }
}
