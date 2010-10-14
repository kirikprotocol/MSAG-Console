#include "RegionFinderV1.h"

namespace eyeline {
namespace informer {

RegionFinderV1::RegionFinderV1() :
log_(smsc::logger::Logger::getInstance("regfinder")) {}


regionid_type RegionFinderV1::findRegion( personid_type subscriber )
{
    uint8_t ton, npi;
    uint64_t addr = subscriberToAddress(subscriber,ton,npi);
    char buf[30];
    sprintf(buf,"%llu",ulonglong(addr));
    regionid_type rid;
    if (!tree_.Find(buf,rid)) {
        rid = defaultRegionId;
    }
    smsc_log_debug(log_,"findRegion(.%u.%u.%llu) -> %u",ton,npi,ulonglong(addr),rid);
    return rid;
}


void RegionFinderV1::updateMasks( Region* regOld, const Region& regNew )
{
    // default region does not have masks
    if ( regNew.getRegionId() == defaultRegionId ) return;

    if ( regOld ) {
        const std::vector< std::string >& oldMasks = regOld->getMasks();
        for ( std::vector< std::string >::const_iterator i = oldMasks.begin();
              i != oldMasks.end(); ++i ) {
            tree_.Delete(i->c_str());
        }
    }
    const std::vector< std::string >& masks = regNew.getMasks();
    for ( std::vector<std::string>::const_iterator i = masks.begin();
          i != masks.end(); ++i ) {
        tree_.Insert(i->c_str(),regNew.getRegionId());
    }
}

}
}
