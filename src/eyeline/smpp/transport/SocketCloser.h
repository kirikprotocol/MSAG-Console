#ifndef _EYELINE_SMPP_SOCKETCLOSER_H
#define _EYELINE_SMPP_SOCKETCLOSER_H

namespace eyeline {
namespace smpp {

class Socket;

class SocketCloser
{
protected:
    virtual ~SocketCloser() {}
public:
    virtual void terminateSocket( Socket& socket ) = 0;
};

}
}


#endif
