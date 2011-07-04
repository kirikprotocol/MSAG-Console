#ifndef _SCAG_TRANSPORT_SMPP_SMPPCOMMANDQUEUE2_H
#define _SCAG_TRANSPORT_SMPP_SMPPCOMMANDQUEUE2_H

#include "SmppChannel2.h"

namespace scag2 {
namespace transport {
namespace smpp {

class SmppCommand;

class SmppCommandQueue{
public:
    virtual void putCommand( SmppChannel& ch, std::auto_ptr<SmppCommand> cmd)=0;
    virtual bool getCommand( SmppCommand*& cmd)=0;
};

}//smpp
}//transport
}//scag

#endif
