#include "Drndm.h"

namespace scag2 {
namespace util {

using namespace smsc::core::synchronization;

Drndm Drndm::rnd;

uint64_t Drndm::getNextNumber()
{
    kr = kr * kg;
    if ( kr >= randmax() ) {
        kr &= ~randmax();
    }
    return kr;
}

uint64_t Drndm::get()
{
    MutexGuard mg(mtx_);
    return getNextNumber();
}


void Drndm::getVector( unsigned howmany, std::vector<uint64_t>& res ) {
    MutexGuard mg(mtx_);
    res.clear();
    res.reserve(howmany);
    for ( ; howmany > 0; --howmany ) {
        res.push_back( getNextNumber() );
    }
}


void Drndm::setSeed( uint64_t seed ) {
    MutexGuard mg(mtx_);
    kr = seed * 2 + 1;
}

}
}
