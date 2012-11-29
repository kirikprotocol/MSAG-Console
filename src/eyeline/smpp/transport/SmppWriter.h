#ifndef _EYELINE_SMPP_SMPPWRITER_H
#define _EYELINE_SMPP_SMPPWRITER_H

#include "SmppIOTask.h"

namespace eyeline {
namespace smpp {

class SmppWriter : public SmppIOTask
{
public:
    SmppWriter( SocketCloser& closer ) : SmppIOTask("smpp.wr",closer), hasData_(false) {
        smsc_log_debug(log_,"r=%u w=%u",wakePipe_.getR(),wakePipe_.getW());
    }

    void wakeUp();

protected:
    int Execute();
    const char* taskName() { return taskName_.c_str(); }
    virtual void postSetSocket( bool add, Socket& sock );
    void innerRemoveSocket( Socket& sock );
private:
    bool hasData_;
};

}
}

#endif
