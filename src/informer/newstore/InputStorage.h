#ifndef _INFORMER_INPUTSTORAGE_H
#define _INFORMER_INPUTSTORAGE_H

#include "informer/data/InputMessageSource.h"
#include "informer/newstore/InputJournal.h"
#include "logger/Logger.h"

namespace eyeline {
namespace informer {

class InfosmeCore;

/// persistent journal for input storage.
class InputStorage : public InputMessageSource
{
public:
    InputStorage( InfosmeCore& core );
    virtual ~InputStorage();

    virtual TransferTask* startTransferTask( TransferRequester& requester,
                                             unsigned           count,
                                             bool               mayDetachRegion );

private:
    smsc::logger::Logger* log_;
    InfosmeCore& core_;
    InputJournal journal_;
};

} // informer
} // smsc

#endif
