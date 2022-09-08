/* NaNDefs.h
 * 
 * Author: Wentao Wu
 * Created: 20200414
*/

#pragma once

#include "NumericTime.h"
#include <cmath>
#include <stdexcept>

namespace wcc {

    template<typename Value> struct GetNaN { };
    template<> struct GetNaN<char              > { static constexpr char                value = '\0';                                               };
    template<> struct GetNaN<short             > { static constexpr short               value = std::numeric_limits<short>::min();                  };
    template<> struct GetNaN<int               > { static constexpr int                 value = std::numeric_limits<int>::min();                    };
    template<> struct GetNaN<long              > { static constexpr long                value = std::numeric_limits<long>::min();                   };
    template<> struct GetNaN<long long         > { static constexpr long long           value = std::numeric_limits<long long>::min();              };
    template<> struct GetNaN<unsigned short    > { static constexpr unsigned short      value = std::numeric_limits<unsigned short>::max() - 1;     };
    template<> struct GetNaN<unsigned          > { static constexpr unsigned            value = std::numeric_limits<unsigned>::max() - 1;           };
    template<> struct GetNaN<unsigned long     > { static constexpr unsigned long       value = std::numeric_limits<unsigned long>::max() - 1;      };
    template<> struct GetNaN<unsigned long long> { static constexpr unsigned long long  value = std::numeric_limits<unsigned long long>::max() - 1; };
    template<> struct GetNaN<float             > { static constexpr float               value = std::numeric_limits<float>::quiet_NaN();            };
    template<> struct GetNaN<double            > { static constexpr double              value = std::numeric_limits<double>::quiet_NaN();           };
    template<> struct GetNaN<wcc::NumericTime  > { inline static const wcc::NumericTime value = wcc::NumericTime::NaN;                              }; // require c++17 

    template<typename Value> inline bool isnan(Value const& v) { return v == GetNaN<Value>::value; }
    template<> inline bool isnan<float >(float  const& v) { return std::isnan(v); }
    template<> inline bool isnan<double>(double const& v) { return std::isnan(v); }

} // namespace wcc
