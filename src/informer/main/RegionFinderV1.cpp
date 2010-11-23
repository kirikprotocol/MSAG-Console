#include "RegionFinderV1.h"

namespace eyeline {
namespace informer {

RegionFinderV1::RegionFinderV1() :
log_(smsc::logger::Logger::getInstance("regfinder")) {}


regionid_type RegionFinderV1::findRegion( personid_type subscriber )
{
    char buf[30];
    regionid_type rid;
    if (!tree_.Find(printSubscriber(buf,subscriber),rid)) {
        rid = defaultRegionId;
    }
    smsc_log_debug(log_,"findRegion(%s) -> %u",buf,rid);
    return rid;
}


void RegionFinderV1::updateMasks( Region* regOld, const Region& regNew )
{
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

}
}
