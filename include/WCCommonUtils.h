/* WCCommonUtils.h
 * 
 * Author: Wentao Wu
*/

#pragma once

#include <cstdlib>
#include <ctime>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>

namespace wcc {

inline std::string get_today_str() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream today_oss;
    today_oss << std::put_time(&tm, "%Y%m%d");
    std::string today_str = today_oss.str();
    return today_str;
}

inline std::string get_time_str() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream time_oss;
    time_oss << std::put_time(&tm, "%H%M%S");
    std::string time_str = time_oss.str();
    return time_str;
}

inline std::string readline(std::istream& source) {
    constexpr int k_line_buffer_size = 128;
    char command_buffer[k_line_buffer_size];
    return command_buffer;
}

inline std::string readline(FILE* source) {
    constexpr int k_line_buffer_size = 128;
    char command_buffer[k_line_buffer_size];
    // parse
    if(std::fgets(command_buffer, k_line_buffer_size, source) == NULL) {
        return "";
    }
    // remove '\n' at tail
    char* newline_ch;
    if((newline_ch = std::strchr(command_buffer, '\n')) != NULL) {
        *newline_ch = '\0';
    }
    return command_buffer;
}

// extract filename from path
inline std::string get_basename(const std::string& pathname) {
    std::vector<char> buffer(pathname.size()+1, '\0');
    std::copy(pathname.begin(), pathname.end(), buffer.begin());
    return basename(buffer.data());
}
// extract directory from path
inline std::string get_dirname(const std::string& pathname) {
    std::vector<char> buffer(pathname.size()+1, '\0');
    std::copy(pathname.begin(), pathname.end(), buffer.begin());
    return dirname(buffer.data());
}
// create folder if the folder not exist
// returns:
//   0: if the folder has already been exist
//   1: if the folder is created
// throw:
//   runtime_error: if not have permission or need recursive create
inline int mkdir_if_not_exist(const std::string& folder) {
    mode_t folder_mode =  S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH; // 0773
    if(mkdir(folder.c_str(), folder_mode) == -1) { 
        if(errno == EEXIST) {
            return 0;
        } else {
            throw std::runtime_error("Create folder " + folder + " failed: " + std::string(strerror(errno)));
        }
    } 
    return 1;
}

inline void check_file_exist(std::string const& name) {
    struct stat buffer;
    bool is_exist = stat(name.c_str(), &buffer) == 0 ;
    if(!is_exist) {
        throw std::invalid_argument("File " + name + " does not exist");
    }
}

inline constexpr mode_t k_file_mode = S_IRUSR | S_IWUSR | S_IRGRP;
inline constexpr mode_t k_dir_mode  = k_file_mode | S_IXUSR | S_IXGRP;
struct FIFOInfo {

    std::string name;
    int fd;

    FIFOInfo(std::string const& fifo_name)
        : name(fifo_name)
    {
        int fifo_status = mkfifo(name.c_str(), k_file_mode);
        if(fifo_status < 0) { 
            if(errno == EEXIST) {
                throw std::runtime_error("FIFOInfo::" + name + " has already been exist");
            } else {
                throw std::runtime_error("FIFOInfo::mkfifo: cannot create fifo " + name);
            }
        }
        fd = open(name.c_str(), O_RDONLY | O_NONBLOCK);
        if(fd == -1) {
            throw std::runtime_error("FIFOInfo::open");
        }
    }

    ~FIFOInfo() {
        close(fd);
        unlink(name.c_str());
    }
};

inline std::string join_string(std::vector<std::string> const& token, char delim = ',') {
    if(token.empty()) {
        return std::string();
    }
    auto iter = token.cbegin();
    std::string res;
    res.reserve(1024);
    res += *(iter++);
    while(iter != token.end()) {
        res += delim;
        res += *(iter++);
    }
    return res;
}

} /* namespace wcc */
