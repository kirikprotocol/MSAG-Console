#ifndef _INFORMER_INPUTJOURNAL_H
#define _INFORMER_INPUTJOURNAL_H

#include "informer/io/FileGuard.h"
#include "informer/io/Typedefs.h"
#include "core/synchronization/Mutex.hpp"

namespace eyeline {
namespace informer {

class InputRegionRecord;

class InputJournal
{
public:
    struct Reader {
        virtual ~Reader() {}
        /// NOTE: this method is invoked at init ONLY!
        virtual void setRecordAtInit( dlvid_type               dlvId,
                                      const InputRegionRecord& rec,
                                      uint64_t                 maxMsgId ) = 0;
        virtual void postInit() = 0;
    };

    InputJournal();

    // read journal at start
    void init( Reader& jr );

    /// return the number of bytes written
    size_t journalRecord( dlvid_type dlvId,
                          const InputRegionRecord& rec,
                          uint64_t maxMsgId );

    void rollOver();

private:
    void readRecordsFrom( const std::string& jpath, Reader& jr );

private:
    smsc::logger::Logger*              log_;
    smsc::core::synchronization::Mutex lock_;
    FileGuard                          jnl_;
    uint32_t                           version_;
};

} // informer
} // smsc

#endif
