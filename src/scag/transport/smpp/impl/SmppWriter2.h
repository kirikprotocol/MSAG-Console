#ifndef __SCAG_TRANSPORT_SMPP_WRITER2_H__
#define __SCAG_TRANSPORT_SMPP_WRITER2_H__

#include "SmppIOBase2.h"

namespace scag2 {
namespace transport {
namespace smpp {

class SmppSocketManager;

class SmppWriter:public SmppIOBase
{
public:
    SmppWriter( const SmppSocketManager& mgr ) : SmppIOBase(), mgr_(&mgr) {}
    const char* taskName(){return "SmppWriter";}
    virtual void onAddSocket(SmppSocket& sock) {
        sock.setMonitor(&mon);
    }
    int Execute();
protected:
    const SmppSocketManager* mgr_;
};

}//smpp
}//transport
}//scag


#endif
