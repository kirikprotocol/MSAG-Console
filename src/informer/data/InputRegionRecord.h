#ifndef _INFORMER_INPUTREGIONRECORD_H
#define _INFORMER_INPUTREGIONRECORD_H

#include "informer/io/Typedefs.h"

namespace eyeline {
namespace informer {

struct InputRegionRecord
{
    regionid_type regionId;
    uint32_t      rfn;
    uint32_t      roff;
    msgid_type    rlast;    // last read msgid
    uint32_t      wfn;      // current write file number
    uint32_t      woff;     // current write offset
    uint32_t      count;    // number of messages per region

    void clear() {
        rlast = rfn = roff = wfn = woff = count = 0;
    }
};

} // informer
} // smsc

#endif
