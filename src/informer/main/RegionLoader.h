#ifndef _INFORMER_REGIONLOADER_H
#define _INFORMER_REGIONLOADER_H

#include "informer/data/Region.h"

namespace eyeline {
namespace informer {

class RegionLoader
{
public:
    RegionLoader( const char* xmlfile,
                  const char* defaultSmscId,
                  regionid_type rid = regionid_type(-2));
    Region* popNext();
private:
    std::vector< Region* > regions_;
};

} // informer
} // smsc

#endif
