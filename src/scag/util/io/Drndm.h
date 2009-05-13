#ifndef _SCAG_UTIL_DRNDM_H
#define _SCAG_UTIL_DRNDM_H

#include "util/int.h"
#include <vector>
#include "core/synchronization/Mutex.hpp"

namespace scag2 {
namespace util {

class Drndm
{
private:
    static Drndm rnd;
    
public:
    static Drndm& getRnd() { return rnd; }

    static inline uint64_t randmax() { return 0x8000000000000000ULL; }

    static inline uint64_t uniform( uint64_t max, uint64_t r ) {
        return uint64_t((double(r)/randmax())*max);
    }

    Drndm() : kr(1), kg(70369817985301ULL) {}

    /// NOTE: not a thread safe method
    uint64_t getNextNumber();

    /// syncronized access
    uint64_t get();
    
    void getVector( unsigned howmany, std::vector<uint64_t>& res );

    void setSeed( uint64_t seed );

private:
    smsc::core::synchronization::Mutex mtx_;
    uint64_t kr;
    uint64_t kg;
};

} // util
} // scag2

namespace scag {
namespace util {
using scag2::util::Drndm;
}
}

#endif /* !_SCAG_UTIL_DRNDM_H */
