/* LogConfig.h
 * Configure spdlog::loggers according to config file
 * 
 * Author: Wentao Wu
*/

#pragma once

#include <string>
#include <spdlog/spdlog.h>
#include <string_view>

namespace wcc {

void config_log(std::string_view config_file);

std::shared_ptr<spdlog::logger> get_logger(std::string_view logger_name);

} /* namespace wcc */
