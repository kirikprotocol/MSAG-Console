#ifndef _INFORMER_INPUTJOURNAL_H
#define _INFORMER_INPUTJOURNAL_H

#include "informer/io/FileGuard.h"
#include "informer/io/Typedefs.h"
#include "InputRegionRecord.h"

namespace eyeline {
namespace informer {

class InputJournal
{
public:
    InputJournal( const std::string& path );

    void journalRecord( dlvid_type dlvId,
                        regionid_type regionId,
                        const InputRegionRecord& rec,
                        uint64_t maxMsgId );

    /// the path to the store root.
    inline const std::string& getStorePath() const { return path_; }

private:
    std::string path_;  // path to storage root directory
    FileGuard   jnl_;
};

} // informer
} // smsc

#endif
