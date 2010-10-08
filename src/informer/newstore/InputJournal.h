#ifndef _INFORMER_INPUTJOURNAL_H
#define _INFORMER_INPUTJOURNAL_H

#include "informer/io/FileGuard.h"
#include "informer/io/Typedefs.h"
#include "InputRegionRecord.h"

namespace eyeline {
namespace informer {

class CommonSettings;

class InputJournal
{
public:
    InputJournal( const CommonSettings& cs );

    void journalRecord( dlvid_type dlvId,
                        regionid_type regionId,
                        const InputRegionRecord& rec,
                        uint64_t maxMsgId );

    inline const CommonSettings& getCS() const { return cs_; }

private:
    const CommonSettings& cs_;
    FileGuard             jnl_;
};

} // informer
} // smsc

#endif
