#ifndef _INFORMER_BINDSIGNAL_H
#define _INFORMER_BINDSIGNAL_H

#include <vector>
#include "informer/io/Typedefs.h"

namespace eyeline {
namespace informer {

struct BindSignal
{
    dlvid_type                   dlvId;
    std::vector< regionid_type > regIds;
    bool                         bind;
};

} // informer
} // smsc

#endif
