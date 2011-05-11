#include "RegionFinderV1.h"
#include "informer/io/InfosmeException.h"

namespace eyeline {
namespace informer {

RegionFinderV1::RegionFinderV1() :
log_(smsc::logger::Logger::getInstance("regfinder")),
treelock_(),
hashlock_()
{}


void RegionFinderV1::findRegion( personid_type subscriber,
                                 RegionPtr& ptr )
{
    char buf[30];
    printSubscriber(buf,subscriber);
    {
        smsc::core::synchronization::MutexGuard mg(treelock_);
        if (!tree_.Find(buf,ptr)) {
            ptr = default_;
        }
    }
    if (!ptr) {
        throw InfosmeException(EXC_LOGICERROR,"Default region is not set");
    }
    smsc_log_debug(log_,"findRegion(%s) -> %u",buf,ptr->getRegionId());
}


bool RegionFinderV1::getRegion( regionid_type regionId, RegionPtr& ptr )
{
    smsc::core::synchronization::MutexGuard mg(hashlock_);
    RegionPtr* p = hash_.GetPtr(regionId);
    if (!p) return false;
    ptr = *p;
    if (!ptr) return false;
    return true;
}


bool RegionFinderV1::updateRegion( RegionPtr& ptr )
{
    if (!ptr) {
        throw InfosmeException(EXC_LOGICERROR,"input region is null");
    }
    const regionid_type regionId = ptr->getRegionId();

    // find/update the old region
    RegionPtr old;
    {
        smsc::core::synchronization::MutexGuard mg(hashlock_);
        RegionPtr* p = hash_.GetPtr(regionId);
        if (!p) {
            hash_.Insert(regionId,ptr);
        } else if ( !*p ) {
            *p = ptr;
        } else {
            old = *p;
        }

    }
    {
        smsc::core::synchronization::MutexGuard mg(treelock_);

        // delete old masks
        if ( old.get() ) {

            if ( regionId == defaultRegionId ) {
                // do nothing
            } else if ( ptr->isDeleted() || !old->hasEqualMasks(*ptr) ) {
                // masks differ
                const std::vector< std::string > masks = old->getMasks();
                for ( std::vector< std::string >::const_iterator i = masks.begin(),
                      ie = masks.end(); i != ie; ++i ) {
                    tree_.Delete(i->c_str());
                }
            }

            // updating region masks is under treelock_
            old->swap( *ptr );
            ptr = old;

        }

        if ( regionId == defaultRegionId ) {
            default_ = ptr;
        } else if ( !ptr->isDeleted() ) {
            const std::vector< std::string > masks = ptr->getMasks();
            for ( std::vector< std::string >::const_iterator i = masks.begin(),
                  ie = masks.end(); i != ie; ++i ) {
                tree_.Insert(i->c_str(),ptr);
            }
        }
    }
    return old.get();
}


void RegionFinderV1::clear()
{
    {
        smsc::core::synchronization::MutexGuard mg(treelock_);
        tree_.Clear();
        default_.reset(0);
    }
    smsc::core::synchronization::MutexGuard mg(hashlock_);
    hash_.Empty();
}


/*
void RegionFinderV1::updateMasks( Region* regOld, const Region& regNew )
{
    smsc::core::synchronization::MutexGuard mg(lock_);
    // default region does not have masks
    if ( regNew.getRegionId() == defaultRegionId ) return;

    if ( regOld ) {
        if ( !regNew.isDeleted() && regOld->hasEqualMasks(regNew) ) return;

        const std::vector< std::string >& oldMasks = regOld->getMasks();
        for ( std::vector< std::string >::const_iterator i = oldMasks.begin();
              i != oldMasks.end(); ++i ) {
            tree_.Delete(i->c_str());
        }
    }

    if ( !regNew.isDeleted() ) {
        const std::vector< std::string >& masks = regNew.getMasks();
        for ( std::vector<std::string>::const_iterator i = masks.begin();
              i != masks.end(); ++i ) {
            tree_.Insert(i->c_str(),regNew.getRegionId());
        }
    }
}
 */

}
}
