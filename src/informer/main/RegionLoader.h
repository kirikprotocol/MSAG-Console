#ifndef _INFORMER_REGIONLOADER_H
#define _INFORMER_REGIONLOADER_H

#include "informer/data/Region.h"
#include "logger/Logger.h"

namespace eyeline {
namespace informer {

class RegionLoader
{
public:
    RegionLoader( const char* xmlfile,
                  const char* defaultSmscId,
                  regionid_type rid = anyRegionId);
    Region* popNext();
private:
    smsc::logger::Logger*  log_;
    std::vector< Region* > regions_;
};

} // informer
} // smsc

#endif
