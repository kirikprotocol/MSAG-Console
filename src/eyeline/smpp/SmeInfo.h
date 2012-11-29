#ifndef _EYELINE_SMPP_SMEINFO_H
#define _EYELINE_SMPP_SMEINFO_H

#include <string>
#include "BindMode.h"

namespace eyeline {
namespace smpp {

struct SmeInfo
{
    std::string systemId;
    std::string password;
    bool        enabled;
    unsigned    nsockets; // how many sockets of this type is allowed
    BindMode    bindMode;
};

}
}

#endif
