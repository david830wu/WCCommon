/* CsvIOTest.cpp
* 
* Author: Wentao Wu
*/

#include "CsvIO.h"
#include <vector>
#include <fmt/format.h>

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

using namespace wcc;

using Depth = std::tuple<
    uint32_t       ,
    NumericTime    ,
    double         ,
    double         ,
    unsigned long  ,
    double         ,
    unsigned long  ,
    char           ,
    NumericTime
>;
struct DepthField {
    constexpr static std::size_t Instrument      = 0;
    constexpr static std::size_t DepthMarketTime = 1;
    constexpr static std::size_t Last            = 2;
    constexpr static std::size_t AskPrice1       = 3;
    constexpr static std::size_t AskVol1         = 4;
    constexpr static std::size_t BidPrice1       = 5;
    constexpr static std::size_t BidVol1         = 6;
    constexpr static std::size_t Status          = 7;
    constexpr static std::size_t HostTime        = 8;
};

#ifdef WCC_USE_CUSTOM_CONVERTER
namespace wcc {
    // read 1<<20 Depth tuples: 
    //   - Default    method: 2.061s
    //   - Use custom method: 0.498s
    // leads a 4X faster
    template<>
    const char* string_to_tuple(const char* line, Depth& tuple, char delim) {
        // line = skip_token(line, delim); 
        line = read_token(line, &std::get<DepthField::Instrument     >(tuple), delim );
        line = read_token(line, &std::get<DepthField::DepthMarketTime>(tuple), delim );
        line = read_token(line, &std::get<DepthField::Last           >(tuple), delim );
        line = read_token(line, &std::get<DepthField::AskPrice1      >(tuple), delim );
        line = read_token(line, &std::get<DepthField::AskVol1        >(tuple), delim );
        line = read_token(line, &std::get<DepthField::BidPrice1      >(tuple), delim );
        line = read_token(line, &std::get<DepthField::BidVol1        >(tuple), delim );
        line = read_token(line, &std::get<DepthField::Status         >(tuple), delim );
        line = read_token(line, &std::get<DepthField::HostTime       >(tuple), '\n'  );
        return line;
    }

    // write 1<<20 Depth tuples: 
    //   - Default    method: 1.859s
    //   - Use custom method: 1.004s
    // leads a 2X faster
    template<>
    int tuple_to_string(char* line, std::size_t size, Depth const& tuple, char delim) {
        int used = 0, total_used = 0;
    #define WRITE_TOKEN(data, delim) \
        used = write_token(line + total_used, size - total_used, (data), (delim) ); total_used = used < 0 ? size : total_used + used;
        WRITE_TOKEN(std::get<DepthField::Instrument     >(tuple), delim );
        WRITE_TOKEN(std::get<DepthField::DepthMarketTime>(tuple), delim );
        WRITE_TOKEN(std::get<DepthField::Last           >(tuple), delim );
        WRITE_TOKEN(std::get<DepthField::AskPrice1      >(tuple), delim );
        WRITE_TOKEN(std::get<DepthField::AskVol1        >(tuple), delim );
        WRITE_TOKEN(std::get<DepthField::BidPrice1      >(tuple), delim );
        WRITE_TOKEN(std::get<DepthField::BidVol1        >(tuple), delim );
        WRITE_TOKEN(std::get<DepthField::Status         >(tuple), delim );
        WRITE_TOKEN(std::get<DepthField::HostTime       >(tuple), '\n'  );
    #undef WRITE_TOKEN
        if(total_used == size) { 
            line[0] = '\0'; 
            return -1; 
        }
        return total_used;
    }
} // namespace wcc
#endif

static const std::size_t k_len = 1<<20;

void generate_depth(std::vector<Depth>& data) {
    for(std::size_t i = 0; i < k_len; ++i) {
        data.push_back(std::make_tuple(
            i+1       , // Instrument      
            133000'000, // DepthMarketTime 
            7.80      , // Last            
            7.80      , // AskPrice1       
            1000      , // AskVol1         
            7.79      , // BidPrice1       
            3100      , // BidVol1         
            'T'       , // Status          
            NumericTime::now() // HostTime        
        ));
    }
}

namespace Catch {
    template<>
    struct StringMaker<Depth> {
        static std::string convert(Depth const& value ) {
            std::string buffer(64, '\0');
            tuple_to_string(buffer.data(), 64, value);
            return buffer.data();
        }
    };
}

std::vector<Depth> depths;

TEST_CASE("CsvIOTest", "[WCCommon]") {
    std::string test_file_name = "CsvIOTest.csv";
    generate_depth(depths);

    SECTION("write") {
        auto write_s = std::chrono::high_resolution_clock::now();
        write_csv(test_file_name, depths);
        auto write_e = std::chrono::high_resolution_clock::now();
        fmt::print("Write Cost = {:.3f}s\n", 
            (double)(std::chrono::duration_cast<std::chrono::milliseconds>(write_e - write_s).count())/1000.0 
        );
    }
    SECTION("read") {
        std::vector<Depth> read_depths;
        auto read_s = std::chrono::high_resolution_clock::now();
        read_csv(test_file_name, read_depths);
        auto read_e = std::chrono::high_resolution_clock::now();
        fmt::print("Read Cost = {:.3f}s, Insert {} items\n", 
            (double)(std::chrono::duration_cast<std::chrono::milliseconds>(read_e - read_s).count())/1000.0 
            , read_depths.size()
        );
        for(std::size_t i = 0; i < k_len; ++i) {
            REQUIRE(read_depths[i] == depths[i]) ;
        }
    }
    SECTION("read with filter") {
        std::vector<Depth> read_depths;
        auto read_s = std::chrono::high_resolution_clock::now();
        read_csv(test_file_name, read_depths, [](Depth const& depth){
            return std::get<DepthField::Instrument>(depth) > (k_len + 1) / 2;
        });
        auto read_e = std::chrono::high_resolution_clock::now();
        fmt::print("Read Cost = {:.3f}s, Insert {} items\n", 
            (double)(std::chrono::duration_cast<std::chrono::milliseconds>(read_e - read_s).count())/1000.0 
            , read_depths.size()
        );
        REQUIRE(read_depths.size() == (k_len/2) );
    }
}
