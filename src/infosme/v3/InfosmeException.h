#ifndef _INFOSME_V3_EXCEPTION_H
#define _INFOSME_V3_EXCEPTION_H

#include "util/Exception.hpp"

namespace smsc {
namespace infosme {

class InfosmeException : public smsc::util::Exception
{
public:
    InfosmeException( const char* fmt, ... )
    {
        SMSC_UTIL_EX_FILL(fmt);
    }
};

}
}

#endif
