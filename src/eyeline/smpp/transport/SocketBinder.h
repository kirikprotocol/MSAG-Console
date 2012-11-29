#ifndef _EYELINE_SMPP_SOCKETBINDER_H
#define _EYELINE_SMPP_SOCKETBINDER_H

namespace eyeline {
namespace smpp {

class Socket;

class SocketBinder
{
protected:
    virtual ~SocketBinder() {}
public:
    virtual void socketIsReadyToBind( Socket& sock ) = 0;
};

}
}

#endif
