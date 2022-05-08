
/* Utils.h
* implement of utils function and classes
*
* Author: Wentao Wu
*/

#pragma once

#include <array>
#include <string>
#include <cstring>
#include <iconv.h>

namespace wcc {

// convert gbk string to utf8 string
class UTF8Convertor {
public:
    static const std::size_t k_buffer_size = 1024;

    UTF8Convertor()
        : convertor_(iconv_open("UTF8", "GB2312")) 
    { }

    ~UTF8Convertor() { 
        iconv_close(convertor_); 
    }

    const char* operator()(const char* gbk_str, const std::size_t& len) {
        char* gbk_pos = const_cast<char*>(gbk_str);
        char* utf_pos = buffer_.data();
        std::size_t gbk_left = len;
        std::size_t utf_left = k_buffer_size;
        size_t rt = iconv(convertor_
            , &gbk_pos, &gbk_left
            , &utf_pos, &utf_left
        );
        if(rt == (size_t)(-1)) {
            std::strncpy(buffer_.data(), "Error: Convert to UTF8 failed", k_buffer_size);
        }
        return buffer_.data();
    }

    std::string operator()(const std::string& gbk_str) {
        std::string utf8_string(operator()(gbk_str.c_str(), gbk_str.size()));
        return utf8_string;
    }

private:
    std::array<char, k_buffer_size> buffer_;
    iconv_t convertor_;
};

} /* namespace wcc */