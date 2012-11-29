#ifndef _EYELINE_SMPP_SMPPREADER_H
#define _EYELINE_SMPP_SMPPREADER_H

#include "SmppIOTask.h"

namespace eyeline {
namespace smpp {

class SmppReader : public SmppIOTask
{
public:
    SmppReader( SocketCloser& closer,
                msectime_type inactlimit ) :
    SmppIOTask("smpp.rd",closer), inactivityLimit_(inactlimit) {
        smsc_log_debug(log_,"r=%u w=%u inactLimit=%u",
                       wakePipe_.getR(),wakePipe_.getW(),unsigned(inactivityLimit_));
    }

protected:
    int Execute();
    virtual void postSetSocket( bool add, Socket& sock );
    std::vector< SocketPtr >::iterator innerRemoveSocket( Socket& sock );

    /// check socket inactivity and return the time of the next check
    msectime_type checkInactivity( msectime_type now );

private:
    msectime_type inactivityLimit_;
};

}
}

#endif
