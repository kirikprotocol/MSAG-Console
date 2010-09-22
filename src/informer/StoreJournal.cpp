#include "StoreJournal.h"
#include "Message.h"

namespace smsc {
namespace informer {

void StoreJournal::journalMessage( dlvid_type     dlvId,
                                   regionid_type  regionId,
                                   const Message& msg )
{
    const size_t bufsize = 200;
    char buf[bufsize];
    char* p = buf;
    p += sprintf(buf,"%u,%u,",dlvId,regionId);
    p = msg.printToBuf(bufsize-(p-buf),p);
    smsc_log_debug(log_,"%s",buf);
}

}
}
