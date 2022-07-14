/* ProgressBar.h
 * 
 * Author: Wentao Wu
*/

#pragma once

#include <iostream>
#include <string>

namespace wcc {

    class ProgressBar {
    public:
        ProgressBar(std::size_t bar_width, std::ostream& os)
            : bar_width_(bar_width)
            , bar_str_(bar_width_ + 1, '\0')
            , prev_prog_(-1)
            , os_(os)
        { }

        // progress is an integer between 0(start) to 100(completed)
        void update(int progress) {
            if(progress < 0  ) progress = 0  ;
            if(progress > 100) progress = 100;

            if(progress > prev_prog_) {
                int pos = bar_width_ * progress / 100;
                for (int i = 0; i < bar_width_; ++i) {
                    if      (i <  pos) bar_str_[i] = '=';
                    else if (i == pos) bar_str_[i] = '>';
                    else               bar_str_[i] = ' ';
                }
                os_ << "[" << bar_str_ << "] " << progress << "%\r";
                os_.flush();
                prev_prog_ = progress;
            }
        }

        void finish() const {
            os_ << std::endl;
        }

    private:
        std::size_t bar_width_;
        std::string bar_str_;
        int prev_prog_;
        std::ostream& os_;

    }; // class ProgressBar

} // namespace wcc 