/* CsvIO.h
 * Fast, simple and user-friendly utils to read and write CSV file
 * 
 * Author: Wentao Wu
*/

#pragma once

#include "NaNDefs.h"
#include "NumericTime.h"
#include "ProgressBar.h"

#include <tuple>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

namespace wcc {

template <typename T>
struct StringToValueFalse { enum { value = false }; };
template<typename ValueType> inline auto string_to_value(const char* buffer) -> ValueType { 
    static_assert(StringToValueFalse<ValueType>::value, "Not Implemented"); 
    return ValueType();
}
template<> inline auto string_to_value<std::string       >(const char* buffer) -> std::string        { return buffer;        }
template<> inline auto string_to_value<bool              >(const char* buffer) -> bool               { return (buffer[0] == 'T') || (buffer[0] == '1'); }
template<> inline auto string_to_value<char              >(const char* buffer) -> char               { return buffer[0];     }
template<> inline auto string_to_value<short             >(const char* buffer) -> short              { return atoi (buffer); }
template<> inline auto string_to_value<int               >(const char* buffer) -> int                { return atoi (buffer); }
template<> inline auto string_to_value<long              >(const char* buffer) -> long               { return atol (buffer); }
template<> inline auto string_to_value<long long         >(const char* buffer) -> long long          { return atoll(buffer); }
template<> inline auto string_to_value<unsigned short    >(const char* buffer) -> unsigned short     { return atoi (buffer); }
template<> inline auto string_to_value<unsigned int      >(const char* buffer) -> unsigned int       { return atol (buffer); }
template<> inline auto string_to_value<unsigned long     >(const char* buffer) -> unsigned long      { return atoll(buffer); }
template<> inline auto string_to_value<float             >(const char* buffer) -> float              { return atof (buffer); }
template<> inline auto string_to_value<double            >(const char* buffer) -> double             { return atof (buffer); }
template<> inline auto string_to_value<unsigned long long>(const char* buffer) -> unsigned long long { return atoll(buffer); }
template<> inline auto string_to_value<wcc::NumericTime  >(const char* buffer) -> wcc::NumericTime   { return wcc::NumericTime(atoi(buffer));}

template <typename T>
struct ValueToStringFalse { enum { value = false }; };
template<typename ValueType> inline int value_to_string(char* buffer, std::size_t size, ValueType const& value) { 
    static_assert(ValueToStringFalse<ValueType>::value, "Not Implemented"); 
    return 0;
}
template<> inline int value_to_string<std::string       >(char* buffer, std::size_t size, std::string        const& value) { return snprintf(buffer, size, "%s"  , value.c_str());}
template<> inline int value_to_string<bool              >(char* buffer, std::size_t size, bool               const& value) { return snprintf(buffer, size, "%c"  , value ? 'T' : 'F'); }
template<> inline int value_to_string<char              >(char* buffer, std::size_t size, char               const& value) { return snprintf(buffer, size, "%c"  , value); }
template<> inline int value_to_string<short             >(char* buffer, std::size_t size, short              const& value) { return snprintf(buffer, size, "%hd" , value); }
template<> inline int value_to_string<int               >(char* buffer, std::size_t size, int                const& value) { return snprintf(buffer, size, "%d"  , value); }
template<> inline int value_to_string<long              >(char* buffer, std::size_t size, long               const& value) { return snprintf(buffer, size, "%ld" , value); }
template<> inline int value_to_string<long long         >(char* buffer, std::size_t size, long long          const& value) { return snprintf(buffer, size, "%lld", value); }
template<> inline int value_to_string<unsigned short    >(char* buffer, std::size_t size, unsigned short     const& value) { return snprintf(buffer, size, "%hu" , value); }
template<> inline int value_to_string<unsigned int      >(char* buffer, std::size_t size, unsigned int       const& value) { return snprintf(buffer, size, "%u"  , value); }
template<> inline int value_to_string<unsigned long     >(char* buffer, std::size_t size, unsigned long      const& value) { return snprintf(buffer, size, "%lu" , value); }
template<> inline int value_to_string<float             >(char* buffer, std::size_t size, float              const& value) { return snprintf(buffer, size, "%.4f", value); }
template<> inline int value_to_string<double            >(char* buffer, std::size_t size, double             const& value) { return snprintf(buffer, size, "%.4f", value); }
template<> inline int value_to_string<unsigned long long>(char* buffer, std::size_t size, unsigned long long const& value) { return snprintf(buffer, size, "%llu", value); }
template<> inline int value_to_string<wcc::NumericTime  >(char* buffer, std::size_t size, wcc::NumericTime   const& value) { return snprintf(buffer, size, "%09u", static_cast<uint32_t>(value));}

inline const char* skip_token(const char* line, char delim) {
    while(*line != delim) {
        if(*line == '\0') {
            throw std::runtime_error("skip_token,MissingTokenEndDelim");
        }
        ++line; 
    }
    ++line;
    return line;
}

// end_delim cannot set to be '\0'
template<typename T>
inline const char* read_token(const char* buffer, T* p_value, char end_delim = ',') {
    constexpr static int k_max_token_size = 32 + 1;
    char token_buffer[k_max_token_size];
    int i;

    if(buffer[0] == end_delim) {
        *p_value = GetNaN<T>::value;
        return buffer + 1;
    }
    for(i = 0; i < k_max_token_size - 1; ++i) {
        if(buffer[i] == end_delim)
            break;
        else if(buffer[i] == '\0') {
            throw std::runtime_error("read_token,MissingTokenEndDelim");
        } 
        token_buffer[i] = buffer[i];
    }
    token_buffer[i] = '\0'; // change end delim to '\0'
    *p_value = string_to_value<T>(token_buffer);
    return buffer + i + 1;
}

// return written char number ('\0' excluded) or -1 if failure
template<typename T>
inline int write_token(char* buffer, std::size_t size, T const& value, char end_delim = ',') {
    constexpr static int k_max_token_size = 32 + 1;
    int used;
    char token_buffer[k_max_token_size];
    if(size < 2)
        return -1;
    used = value_to_string<T>(token_buffer, k_max_token_size, value);
    if((used < 0) || (used + 2 > size)) {
        buffer[0] = '\0'; // write nothing;
        return -1;
    } else {
        std::memcpy(buffer, token_buffer, used);
        buffer[used] = end_delim;
        buffer[used + 1] = '\0';
        return used + 1;
    }
}

// Map file to read-only memory segment, efficient for open large files
class MmapFile {
public:
    MmapFile(std::string const& file_name)
        : file_name_(file_name)
        , page_size_(sysconf(_SC_PAGESIZE))
    {
        int fd = open(file_name_.c_str(), O_RDONLY);
        if(fd == -1) {
            throw std::runtime_error("Failed to open file: " + file_name_);
        }
        struct stat sb;
        if(fstat(fd, &sb) == -1) {
            throw std::runtime_error("Failed to obtain the file size: " + file_name_);
        }
        size_ = sb.st_size;
        capacity_ = (size_ / page_size_ + 1) * page_size_;
        addr_ = (char*)mmap(NULL, capacity_, PROT_READ, MAP_PRIVATE, fd, 0);
        if(addr_ == MAP_FAILED) {
            throw std::runtime_error("Failed to map file to memory: " + file_name_);
        }
        close(fd);
    }
    ~MmapFile() {
        if(munmap(addr_, capacity_) == -1) {
            std::cerr << "Failed to munmap file: " << file_name_ << std::endl;
        }
    }

    const char* begin()    const noexcept { return addr_;        }
    const char* end()      const noexcept { return addr_+ size_; }
    const char* cbegin()   const noexcept { return addr_;        }
    const char* cend()     const noexcept { return addr_+ size_; }
    std::size_t size()     const noexcept { return size_;        }
    std::size_t capacity() const noexcept { return capacity_;    }

    const char* data() const noexcept {
        return addr_;
    }

private:
    std::string file_name_;
    std::size_t page_size_;
    std::size_t size_     ;
    std::size_t capacity_ ;
    char* addr_;
};

template<typename T>
inline void read_field(std::stringstream& ss, char delim, T& value) {
    std::string value_str;
    std::getline(ss, value_str, delim);
    value = string_to_value<T>(value_str.c_str());
}

// Caution: Default impl has a poor performance on large file.
// It is strongly RECOMMANDED to define a specification of this template function for custom types.
template<typename... Args>
inline const char* string_to_tuple(const char* line, std::tuple<Args...>& tuple, char delim = ',') {
    const char* end_pos = line;
    while(*end_pos != '\n') {
        if(*end_pos == '\0') {
            std::runtime_error("string_to_tuple,MissingLineEnd");
        }
        ++end_pos;
    }
    std::string line_buffer(line, end_pos);
    std::stringstream ss(line_buffer);
    std::string _;
    // std::getline(ss, _, ','); // skip first field
    std::apply([&ss, delim](Args&... args) {
        ((read_field(ss, delim, args)), ...);
    }, tuple);
    return line + line_buffer.size() + 1;
}


template<typename T>
inline std::string to_string_safe(const T& value) {
    if constexpr (std::is_same_v<T, char>) {
        return std::string(1, value);
    } else {
        return std::to_string(value);
    }
}

// Caution: Default impl has a poor performance on large file.
// It is strongly RECOMMANDED to define a specification of this template function for custom types.
template<typename... Args>
inline int tuple_to_string(char* line, std::size_t size, std::tuple<Args...> const& tuple, char delim = ',') {
    std::stringstream ss;
    std::apply([&ss, delim](Args const&... args) {
        ((ss << to_string_safe(args) << delim), ...);
    }, tuple);
    std::string buffer = ss.str();
    if(buffer.size() > size || buffer.empty()) {
        line[0] = '\0';
        return -1;
    } else {
        *buffer.rbegin() = '\n';
        std::strncpy(line, buffer.data(), buffer.size());
        return buffer.size();
    }
}

// add read tuple to container if predicate == true
template<typename Container, typename Pred>
inline void read_csv(std::string const& filename, Container& data, Pred pred) {
    using value_type = typename Container::value_type;
    MmapFile mmap_file(filename);
    const char* buffer_begin = mmap_file.begin();
    const char* buffer_end   = mmap_file.end()  ;
    std::size_t buffer_size  = mmap_file.size() ;
    const char* buffer = buffer_begin;

    data.clear();
    value_type element;
    ProgressBar pbar(70, std::cout);
    while(buffer < buffer_end) {
        buffer = string_to_tuple(buffer, element, ',');
        pbar.update((buffer - buffer_begin) * 100 / buffer_size);
        if(pred(element)) {
            data.push_back(element);
        }
    }
    pbar.finish();
}

template<typename Container>
inline void read_csv(std::string const& filename, Container& data) {
    using value_type = typename Container::value_type;
    read_csv(filename, data, [](const value_type&) { return true; });
}

template<typename Container>
inline void write_csv(std::string const& filename, Container const& data) {
    const static std::size_t k_buffer_size = 64UL << 20; // 64MB buffer
    using value_type = typename Container::value_type;

    std::ofstream of(filename);
    std::vector<char> buffer_a(k_buffer_size, '\0');
    int used = 0;
    std::size_t total_used = 0;
    std::size_t len = data.size();
    ProgressBar pbar(70, std::cout);
    for(std::size_t i = 0; i < len; ++i) {
        used = tuple_to_string(buffer_a.data() + total_used, k_buffer_size - total_used, data[i], ',');
        if(used < 0) {
            of.write(buffer_a.data(), total_used); // write buffer to file
            used = tuple_to_string(buffer_a.data(), k_buffer_size, data[i], ','); // retry with emtpy buffer
            if(used < 0) { throw std::runtime_error("Buffer is not enough for a single tuple"); }
            total_used = used;
        } else {
            total_used += used;
        }
        pbar.update( (i + 1) * 100 / len );
    }
    buffer_a[total_used] = '\0';
    of.write(buffer_a.data(), total_used);
    pbar.finish();
    of.close();
}

} // namespace wcc 