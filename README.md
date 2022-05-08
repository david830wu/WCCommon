# WCCommon Tools

## Intro

WCCommon is a collection of frequently used functions and tools in HFT development.

## Utils

### LogConfig

A convenient function to configure loggers in spdlog according to an yaml file.

```yaml
%YAML 1.2
---
default_format    : "[%m-%d %H:%M:%S.%f] [%-8l] [%-12n] %v"
default_level     : "info"
default_log_dir   : "./log/${today}"
default_log_prefix: "WCCommon"

sinks:
  - stdout
  - basic_file

loggers:
  - main 
  - default

set_error_loggers:
  - default 

set_debug_loggers:
  - default
```

And loggers are created and configured as
```cpp
int main() {
    wcc::config_log("LogConfig.yaml");
    auto p_logger = spdlog::get("main");
    p_logger->debug("This is a {} message", "debug");
    p_logger->info("pi = {:.4f}", 3.1415926);
    p_logger->warn("The answer is {:6d}", 42);
    p_logger->error("{:06d} is SZ stock", 5);
    p_logger->critical("{} is not a {}", "cat", "fruit");
    return 0;
}
```


### NumericTime

An fast and handy class to store and calculate trading time

```cpp
#include "NumericTime.h"

using wcc::NumericTime;

TEST_CASE("NumericTimeTest", "[NumericTime]") {
    SECTION("InitInt") {
        REQUIRE(NumericTime(103159010) == NumericTime(10, 31, 59, 10));
    }
    SECTION("InitStr") {
        REQUIRE(NumericTime("10:31:59.010") == NumericTime(10, 31, 59, 10));
    }
    SECTION("Invalid convert") {
        REQUIRE(NumericTime(23, 59, 59, 1000)== NumericTime(0, 0, 0, 0));
    }
    SECTION("GetField") {
        auto ntime = NumericTime(10, 31, 59, 10);
        REQUIRE(ntime.hour() == 10);
        REQUIRE(ntime.min()  == 31);
        REQUIRE(ntime.sec()  == 59);
        REQUIRE(ntime.ms()   == 10);
    }
    SECTION("ToStr") {
        REQUIRE(NumericTime(10, 31, 59, 10).str() == "10:31:59.010");
    }
    SECTION("Negative time") {
        REQUIRE(NumericTime(10, 31, 59, -1) == NumericTime(10, 31, 58, 999));
        REQUIRE(NumericTime(10, 31, -1, 10) == NumericTime(10, 30, 59,  10));
        REQUIRE(NumericTime(10, -1, 59, 10) == NumericTime( 9, 59, 59,  10));
        REQUIRE(NumericTime(-1, 31, 59, 10) == NumericTime(23, 31, 59,  10));
    }
    SECTION("Now") {
        NumericTime now_time = NumericTime::now();
        std::string now_str = now_time.str();
        std::cout << "Now = " << now_str << std::endl;
    }
}
```

### YAMLGetField

Get field of a given yaml file, replace "${today}" string as its value, and print clear error message if field not exist or type bad conversion.

```yaml
%YAML 1.2
---
data_folder : Data/${today}
trader_id   : 42
username    : username
server_ip   : 127.0.0.1
server_port : 8888
```

Get field with code
```cpp
#include "YAMLGetField.h"

TEST_CASE("YAMLGetFieldTest", "[WCCommon]") {
    SECTION("YAMLGetField") {
        std::string config_file("YAMLGetFieldTest.yaml");
        wcc::check_file_exist(config_file);
        std::string today_str { wcc::get_today_str() };
        YAML::Node config = YAML::LoadFile(config_file);
        std::string data_folder; YAML_GET_FIELD(data_folder, config, data_folder);
        uint64_t    trader_id  ; YAML_GET_FIELD(trader_id  , config, trader_id  );
        std::string username   ; YAML_GET_FIELD(username   , config, username   );
        std::string server_ip  ; YAML_GET_FIELD(server_ip  , config, server_ip  );
        uint32_t    server_port; YAML_GET_FIELD(server_port, config, server_port);
        REQUIRE(data_folder == fmt::format("Data/{}", today_str));
        REQUIRE(trader_id   == 42);
        REQUIRE(username    == "username");
        REQUIRE(server_ip   == "127.0.0.1");
        REQUIRE(server_port == 8888);
    }
}
```

### UTF8Convertor

Convert GBK strings to UTF8 encoding strings;

### WCCommonUtils

```cpp
namespace wcc {
    // get today string as format YYYYMMDD
    inline std::string get_today_str() ;
    // get current time string as format HHMMSS
    inline std::string get_time_str() ;
    // create folder if the folder not exist
    // returns:
    //   0: if the folder has already been exist
    //   1: if the folder is created
    // throw:
    //   runtime_error: if not have permission or need recursive create
    inline int mkdir_if_not_exist(const std::string& folder) ;
    // check if file exist
    // throw:
    //   runtime_error: if file not exist or not have permission to read
    inline void check_file_exist(std::string const& name) ;
    // create and management *nix FIFO
    class FIFOInfo;
}
```



## Dependencies

1. spdlog: https://github.com/gabime/spdlog.git
2. yaml-cpp: https://github.com/jbeder/yaml-cpp.git
3. fmt: https://github.com/fmtlib/fmt.git
