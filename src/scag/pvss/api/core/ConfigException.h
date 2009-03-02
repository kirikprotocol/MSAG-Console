#ifndef _SCAG_PVSS_CORE_CONFIGEXCEPTION_H
#define _SCAG_PVSS_CORE_CONFIGEXCEPTION_H

#include "scag/pvss/common/PvssException.h"

namespace scag2 {
namespace pvss {
namespace core {

class ConfigException : public PvssException
{
public:
    ConfigException( const char* fmt, ... ) :
    PvssException(CONFIG_INVALID) {
        SMSC_UTIL_EX_FILL(fmt);
    }
};

} // namespace core
} // namespace pvss
} // namespace scag2

namespace scag {
namespace pvss {
namespace core {

using scag2::pvss::core::ConfigException;

}
}
}

#endif /* !_SCAG_PVSS_CORE_CONFIGEXCEPTION_H */
