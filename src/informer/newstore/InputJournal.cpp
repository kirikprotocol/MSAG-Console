#include "InputJournal.h"

namespace eyeline {
namespace informer {

InputJournal::InputJournal( const std::string& path ) :
path_(path)
{
    // FIXME: reading journal, move this to init()
    jnl_.create((path_ + "input/.journal").c_str(),true,true);
    const char* head = "# D,R,RFN,ROFF,WFN,WOFF,CNT,MSGID\n";
    jnl_.write(head,strlen(head));
}


void InputJournal::journalRecord( dlvid_type dlvId,
                                  regionid_type regId,
                                  const InputRegionRecord& rec,
                                  uint64_t maxMsgId )
{
    char buf[100];
    sprintf(buf,"%u,%u,%u,%u,%u,%u,%u,%llu\n",
            unsigned(dlvId),unsigned(regId),
            unsigned(rec.rfn),unsigned(rec.roff),
            unsigned(rec.wfn),unsigned(rec.woff),
            unsigned(rec.count),ulonglong(maxMsgId));
    jnl_.write(buf,strlen(buf));
    jnl_.fsync();
}


}
}
