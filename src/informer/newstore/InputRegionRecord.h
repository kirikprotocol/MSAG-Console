#ifndef _INFORMER_INPUTREGIONRECORD_H
#define _INFORMER_INPUTREGIONRECORD_H

#include "util/int.h"

namespace eyeline {
namespace informer {

struct InputRegionRecord
{
    uint32_t  rfn;
    uint32_t  roff;
    uint32_t  wfn;      // current write file number
    uint32_t  woff;     // current write offset
    uint32_t  count;    // number of messages per region

    void clear() {
        rfn = roff = wfn = woff = count = 0;
    }
};

} // informer
} // smsc

#endif
