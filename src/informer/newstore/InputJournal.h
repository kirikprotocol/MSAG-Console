#ifndef _INFORMER_INPUTJOURNAL_H
#define _INFORMER_INPUTJOURNAL_H

#include "informer/io/Typedefs.h"

namespace eyeline {
namespace informer {

/// persistent journal for input storage.
class InputJournal
{
public:
    /// this method is invoked when TransferTask is finishing its work.
    void journalFileOffset( dlvid_type    dlvId,
                            regionid_type regionId,
                            uint32_t      fileNumber,
                            uint32_t      fileOffset );
};

} // informer
} // smsc

#endif
