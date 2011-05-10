#ifndef _INFORMER_REGIONFINDERV1_H
#define _INFORMER_REGIONFINDERV1_H

#include "logger/Logger.h"
#include "informer/data/Region.h"
#include "core/buffers/XTree.hpp"
#include "core/synchronization/Mutex.hpp"

namespace eyeline {
namespace informer {

class RegionFinderV1 : public RegionFinder
{
public:
    RegionFinderV1();
    virtual regionid_type findRegion( personid_type subscriber );
    void updateMasks( Region* oldReg, const Region& regNew );
private:
    smsc::logger::Logger* log_;
    smsc::core::synchronization::Mutex        lock_;
    smsc::core::buffers::XTree<regionid_type> tree_;
};

} // informer
} // smsc

#endif
