#ifndef _INFORMER_REGIONFINDERV1_H
#define _INFORMER_REGIONFINDERV1_H

#include "logger/Logger.h"
#include "informer/data/Region.h"
#include "core/buffers/XTree.hpp"
#include "core/buffers/IntHash.hpp"
#include "core/synchronization/Mutex.hpp"

namespace eyeline {
namespace informer {

class RegionFinderV1 : public RegionFinder
{
public:
    RegionFinderV1();

    /// find a region
    virtual void findRegion( personid_type subscriber, RegionPtr& ptr );

    /// get the region by id
    /// NOTE: that region may be deleted
    virtual bool getRegion( regionid_type regionId, RegionPtr& ptr );

    /// update the region
    /// @return true if region was new, false if it was only updated.
    /// NOTE: ptr may be changed!
    bool updateRegion( RegionPtr& ptr );

    // void updateMasks( Region* oldReg, const RegionPtr& regNew );

    // destroy all regions
    void clear();

private:
    smsc::logger::Logger* log_;

    smsc::core::synchronization::Mutex        treelock_;
    smsc::core::buffers::XTree< RegionPtr >   tree_;
    RegionPtr                                 default_;

    smsc::core::synchronization::Mutex        hashlock_;
    smsc::core::buffers::IntHash< RegionPtr > hash_;
};

} // informer
} // smsc

#endif
