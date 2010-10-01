#include "InputStorage.h"
#include "informer/data/InfosmeCore.h"

namespace eyeline {
namespace informer {

InputStorage::InputStorage( InfosmeCore& core ) :
log_(smsc::logger::Logger::getInstance("instore")),
core_(core)
{
    smsc_log_debug(log_,"ctor");
}


InputStorage::~InputStorage()
{
    smsc_log_debug(log_,"dtor");
}


TransferTask* InputStorage::startTransferTask( TransferRequester& requester,
                                               unsigned           count,
                                               bool mayDetachRegion )
{
    smsc_log_debug(log_,"FIXME: start transfer task");
    return 0;
}


}
}
