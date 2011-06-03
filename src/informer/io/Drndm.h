#ifndef _INFORMER_IO_DRNDM_H
#define _INFORMER_IO_DRNDM_H

#include "util/int.h"

namespace eyeline {
namespace informer {

class Drndm
{
public:
    static inline uint64_t randmax() { return 0x8000000000000000ULL; }

    static inline uint64_t uniform( uint64_t max, uint64_t r ) {
        return uint64_t((double(r)/randmax())*max);
    }

    Drndm() : kr(1), kg(70369817985301ULL) {}

    /// NOTE: not a thread safe method
    uint64_t getNextNumber() {
        kr = kr * kg;
        if ( kr >= randmax() ) {
            kr &= ~randmax();
        }
        return kr;
    }

    void setSeed( uint64_t seed ) {
        kr = seed*2+1;
    }

private:
    uint64_t kr;
    uint64_t kg;
};

} // util
} // scag2

#endif /* !_SCAG_UTIL_DRNDM_H */
