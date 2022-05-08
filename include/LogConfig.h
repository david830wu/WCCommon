/* LogConfig.h
 * Configure spdlog::loggers according to config file
 * 
 * Author: Wentao Wu
*/

#pragma once

#include <string>
#include <spdlog/spdlog.h>

namespace wcc {

void config_log(const std::string& config_file);

} /* namespace wcc */
