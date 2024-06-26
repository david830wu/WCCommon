# WCCommon Tools

## Intro

WCCommon is a collection of frequently used functions and tools in HFT development using using C++20 and later.

## Install

It is recommended to compile using g++-13 or newer compilers.

install WCCommon lib to path ~/opt/WCCommon

```bash
git clone git@github.com:david830wu/WCCommon.git
cd WCCommon
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j8
cmake --install . --prefix ~/opt/WCCommon

# add lib install path to cmake search list
export WCCommon_DIR=~/opt/WCCommon/lib/cmake/WCCommon
```

When MyApp is build with WCCommon, add dependent targets in CMakeLists.txt
```
find_package(WCCommon)
add_executable(MyApp)
target_sources(MyApp PRIVATE main.cpp)
target_link_libraries(MyApp PRIVATE WCCommon::WCCommon WCCommon::LogConfig)
```

## Utils

### CsvIO

Fast, simple and user-friendly utils to read and write CSV file, very efficient for large file.
Watch the progress bar and leave off with confidence.

```cpp
using Depth = std::tuple<
    uint32_t        ,
    wcc::NumericTime,
    double          ,
    double          ,
    unsigned long   ,
    double          ,
    unsigned long   ,
    char            ,
    wcc::NumericTime
>;

static constexpr std::size_t k_len = 1<<20;
std::vector<Depth> depths;

TEST_CASE("CsvIOTest", "[WCCommon]") {
    std::string test_file_name = "CsvIOTest.csv";
    generate_depth(depths, k_len);

    SECTION("write") {
        write_csv(test_file_name, depths);
    }
    SECTION("read") {
        std::vector<Depth> read_depths;
        read_csv(test_file_name, read_depths);
    }
    SECTION("read with filter") {
        std::vector<Depth> read_depths;
        read_csv(test_file_name, read_depths, [](Depth const& depth){
            return std::get<DepthField::Instrument>(depth) > (k_len + 1) / 2;
        });
    }
}
```

### H5IO

Easy to understand and convenient to call function to command hdf5 file read, write and query operations.

```cpp
TEST_CASE("H5IOTest", "[WCCommon]") {
    std::string filename = "H5IOTestData.h5";
    std::vector<double> darray{1.1,1.1,2.1,3.1,5.1,8.1,11.1,13.1};
    std::vector<int   > iarray{1,1,2,3,5,8,11,13};

    SECTION("write") {
        H5File h5_file(filename, 'w');
        hid_t group_id = h5_make_group_if_not_exist(h5_file.id(), "Data");
        h5_write_vector(h5_file.id(), "/Data/d_dataset", darray);  // abs path
        h5_write_vector(group_id    , "i_dataset"      , iarray);  // rel path
        H5Gclose(group_id);
    }
    SECTION("Query") {
        H5File h5_file(filename, 'w');
        REQUIRE(h5_has_object(h5_file.id(), "Data") == true);            // group exist
        REQUIRE(h5_has_object(h5_file.id(), "/Data/d_dataset") == true); // dataset exist
        std::vector<std::string> objs = h5_scan_path_object(h5_file.id(), "Data");
        REQUIRE(objs.size() == 2);
        REQUIRE(objs[0] == "d_dataset");
        REQUIRE(objs[1] == "i_dataset");
        std::vector<std::size_t> dims = h5_query_dataset_dim(h5_file.id(), "/Data/d_dataset");
        REQUIRE(dims.size() == 1);
        REQUIRE(dims[0] == dlist.size());
    }
    SECTION("Read") {
        H5File h5_file(filename, 'r');
        std::vector<double> darray_load;
        std::vector<int   > iarray_load;
        h5_read_vector(h5_file.id(), "/Data/d_dataset", darray_load);
        h5_read_vector(h5_file.id(), "/Data/i_dataset", iarray_load);
        REQUIRE(darray_load == darray);
        REQUIRE(iarray_load == iarray);
    }
}
```

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
#include "LogConfig.h"

int main() {
    wcc::config_log("LogConfig.yaml");
    auto p_logger = wcc::get_logger("main");
    p_logger->debug("This is a {} message", "debug");
    p_logger->info("pi = {:.4f}", 3.1415926);
    p_logger->warn("The answer is {:6d}", 42);
    p_logger->error("{:06d} is SZ stock", 5);
    p_logger->critical("{} is not a {}", "cat", "fruit");
    return 0;
}
```

And log with HJ format can be used with predefined macros
```cpp
#include "HJLogFormat.h"

// Struct with an attached logger named "Test"
struct Logged : wcc::AttachLogger<"Test"> {
   Logged() {
       // Normal usage:
       // Since we are running inside Catch2 internal functions, which cannot
       // be detected by our macro as HAS_ID, the id is output as [-].
       // However, we have defined the id() method for this class.
       ID_MLOG(info, "init", ("Logged constructor called!"));
   }

   void run() {
       // For a variable. All these output without an id field (since using HJ_LOG)
       int x = 1;
       MLOG(info,  "event", VAR(x));           // default format: "x:{}"
       MLOG(error, "event", ("x:{:06}", x));  // specify a format fully

       // Multiple format/value pairs allowed
       // This one also outputs an id field as [1] since we have the id() method for this class
       ID_MLOG(info, "with_id", VAR(x), ("msg:{}", "hello"), ("pi:{}", 3.14));
   }

   auto id() const { return 8; }
};
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

### ProgressBar
Show a progress bar to happily wait for a result.

```cpp
TEST_CASE("ProgressBarTest", "[WCCommon]") {
    SECTION("show") {
        std::size_t data_len = 128;
        wcc::ProgressBar prog_bar(70, std::cout);
        for(std::size_t i = 0; i < data_len ; ++i) {
            // proccessing data
            std::this_thread::sleep_for(std::chrono::milliseconds(20));

            prog_bar.update( (i + 1) * 100 / data_len ); // input ranges from 0(start) to 100(completed)
        }
        prog_bar.finish();
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
format_time : 10:17:19.123
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
        std::string      data_folder; YAML_GET_FIELD(data_folder, config, data_folder);
        uint64_t         trader_id  ; YAML_GET_FIELD(trader_id  , config, trader_id  );
        std::string      username   ; YAML_GET_FIELD(username   , config, username   );
        std::string      server_ip  ; YAML_GET_FIELD(server_ip  , config, server_ip  );
        uint32_t         server_port; YAML_GET_FIELD(server_port, config, server_port);
        wcc::NumericTime format_time; YAML_GET_FIELD(format_time, config, format_time);
        REQUIRE(data_folder == fmt::format("Data/{}", today_str));
        REQUIRE(trader_id   == 42);
        REQUIRE(username    == "username");
        REQUIRE(server_ip   == "127.0.0.1");
        REQUIRE(server_port == 8888);
        REQUIRE(format_time == wcc::NumericTime(10, 17, 19, 123));
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

### DefEnum

DefEnum is a macro to define enum with reflection easily.

```cpp
namespace test {
// A test example here
DEF_ENUM(logout_reason_t, uint8_t,
    (user_initiated_logout,   1)
    (system_initiated_logout)   // this will be 2
    (heartbeat_failure,       101)
)
}  // namespace test

TEST_CASE("enum values", "[DEF_ENUM]") {
    REQUIRE(uint8_t(test::logout_reason_t::user_initiated_logout) == 1);
    REQUIRE(uint8_t(test::logout_reason_t::system_initiated_logout) == 2);
    REQUIRE(uint8_t(test::logout_reason_t::heartbeat_failure) == 101);
}

TEST_CASE("enum strings", "[DEF_ENUM]") {
    REQUIRE(stringize(test::logout_reason_t::user_initiated_logout) == "user_initiated_logout");
    REQUIRE(stringize(test::logout_reason_t::system_initiated_logout) == "system_initiated_logout");
    REQUIRE(stringize(test::logout_reason_t::heartbeat_failure) == "heartbeat_failure");
}
```

### DefTuple

DefEnum is a macro to define std::tuple with reflection easily.

```cpp
namespace test {
DEF_TUPLE(
    Transaction,
    (int, SeqNo)         // unique in one channel
    (double, Price, 6)   // Price1, Price2, ..., Price5
    (int, Volume, 6),    // Volume1, Volume2, ..., Volume5
    trans_field_names
)
}  // namespace test

TEST_CASE("Usages", "[DEF_TUPLE]") {
	test::Transaction t{};  // all zeros

	REQUIRE(test::stringize(t) ==  // everything's zero
        "SeqNo:0, "
        "Price1:0, Price2:0, Price3:0,Price4:0, Price5:0, " 
        "Volume1:0, Volume2:0, Volume3:0, Volume4:0, Volume5:0");

    std::get<test::TransactionField::SeqNo>(t) = 1;
    std::get<test::TransactionField::Price1>(t) = 10000;
    std::get<test::TransactionField::Price2>(t) = 20000;
    std::get<test::TransactionField::Price3>(t) = 30000;
    std::get<test::TransactionField::Price4>(t) = 40000;
    std::get<test::TransactionField::Price5>(t) = 50000;
    std::get<test::TransactionField::Volume1>(t) = 101;
    std::get<test::TransactionField::Volume2>(t) = 201;
    std::get<test::TransactionField::Volume3>(t) = 301;
    std::get<test::TransactionField::Volume4>(t) = 401;
    std::get<test::TransactionField::Volume5>(t) = 501;

    REQUIRE(test::stringize(t) ==
        "SeqNo:1, "
        "Price1:10000, Price2:20000, Price3:30000, Price4:40000, Price5:50000, "
        "Volume1:101, Volume2:201, Volume3:301, Volume4:401, Volume5:501");

    auto const& names = test::trans_field_names(); // names is a ref to a invisible static global vector
    REQUIRE(names[0]  == "SeqNo");
    REQUIRE(names[1]  == "Price1");
    REQUIRE(names[2]  == "Price2");
    REQUIRE(names[3]  == "Price3");
    REQUIRE(names[4]  == "Price4");
    REQUIRE(names[5]  == "Price5");
    REQUIRE(names[6]  == "Volume1");
    REQUIRE(names[7]  == "Volume2");
    REQUIRE(names[8]  == "Volume3");
    REQUIRE(names[9]  == "Volume4");
    REQUIRE(names[10] == "Volume5");
}

```


## Dependencies

1. Catch2: https://github.com/catchorg/Catch2
2. date: https://github.com/HowardHinnant/date
3. fmt: https://github.com/fmtlib/fmt.git
4. libhdf5: https://www.hdfgroup.org/downloads/hdf5
5. spdlog: https://github.com/gabime/spdlog.git
6. yaml-cpp: https://github.com/jbeder/yaml-cpp.git
7. boost: https://www.boost.org
