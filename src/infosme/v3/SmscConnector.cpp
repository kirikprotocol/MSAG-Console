#include "SmscConnector.h"

namespace smsc {
namespace infosme {

/*
int SmscConnector::send( msgtime_type currentTime, Delivery& dlv,
                         regionid_type regionId, Message& msg )
{
    // FIXME: remove this temporary fail
    if ( msg.msgId % 30 == 7 ) {
        return 0;
    } else if ( msg.msgId % 50 == 19 ) {
        throw smsc::util::Exception("message R=%u/D=%u/M=%u cannot be sent",
                                    unsigned(regionId),
                                    unsigned(dlv.getDlvId()),
                                    unsigned(msg.msgId));
    }
    DlvRegMsgId dlvmsg;
    dlvmsg.dlvId = dlv.getDlvId();
    dlvmsg.regId = regionId;
    dlvmsg.msgId = msg.msgId;
    smsc_log_debug(log_,"msg R=%u/D=%u/M=%u is sent",
                   unsigned(dlvmsg.regId),
                   unsigned(dlvmsg.dlvId),
                   unsigned(dlvmsg.msgId) );
    // FIXME
    scheduleResponse( currentTime + msg.msgId % 5 + 3, dlvmsg );
    return 1;
}
 */

virtual int SmscConnector::Execute()
{
}


}
}
