#ifndef _INFORMER_EXCEPTION_H
#define _INFORMER_EXCEPTION_H

#include "util/Exception.hpp"

namespace smsc {
namespace informer {

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
