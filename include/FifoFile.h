#pragma once

#include <string>
#include <sys/stat.h>  // S_IRUSR etc
#include <sys/types.h> // mode_t etc
#include <unistd.h>    // open

class FifoFile {
  public:
    template <typename Str> FifoFile(const Str &fn) : fn_{fn} {
        if (!std::filesystem::exists(fn_)) {
            constexpr auto mode = mode_t(S_IRUSR | S_IWUSR | S_IRGRP);
            if (mkfifo(fn_.c_str(), mode) < 0) {
                throw std::runtime_error(
                    "FifoFile " + fn_ + " cannot be created; " +
                    strerror(errno));
            }
        } else {
            if (!std::filesystem::is_fifo(fn_)) {
                throw std::runtime_error(
                    "FifoFile " + fn_ +
                    " exists but not FIFO! You may need to remove it first.");
            }
        }

        fd_ = open(fn_.c_str(), O_RDONLY | O_NONBLOCK);
        if (fd_ == -1) {
            throw std::runtime_error("FifoFile on opening" + fn_);
        }
    }

    ~FifoFile() {
        close(fd_);
        unlink(fn_.c_str());
    }

    operator int() { return fd_; }

  private:
    std::string fn_;
    int fd_;
};
