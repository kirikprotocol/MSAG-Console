#ifndef _INFORMER_INPUTJOURNAL_H
#define _INFORMER_INPUTJOURNAL_H

#include "informer/io/FileGuard.h"
#include "informer/io/Typedefs.h"
#include "InputRegionRecord.h"
#include "core/synchronization/Mutex.hpp"

namespace eyeline {
namespace informer {

class CommonSettings;

class InputJournal
{
public:
    struct Reader {
        virtual ~Reader() {}
        /// NOTE: this method is invoked at init ONLY!
        virtual void setRecordAtInit( dlvid_type               dlvId,
                                      regionid_type            regionId,
                                      const InputRegionRecord& rec,
                                      uint64_t                 maxMsgId ) = 0;
    };

    InputJournal( const CommonSettings& cs );

    inline const CommonSettings& getCS() const { return cs_; }

    // read journal at start
    void init( Reader& jr );

    void journalRecord( dlvid_type dlvId,
                        regionid_type regionId,
                        const InputRegionRecord& rec,
                        uint64_t maxMsgId );

private:
    void readRecordsFrom( const std::string& jpath, Reader& jr );

private:
    const CommonSettings&              cs_;
    // smsc::logger::Logger*              log_;
    smsc::core::synchronization::Mutex lock_;
    FileGuard                          jnl_;
};

} // informer
} // smsc

#endif
