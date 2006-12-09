#ident "$Id$"
#ifndef __SMSC_INMAN_CONNECT_DEFS_HPP
#define __SMSC_INMAN_CONNECT_DEFS_HPP

#include "core/network/Socket.hpp"
using smsc::core::network::Socket;

namespace smsc  {
namespace inman {
namespace interaction  {

/* ************************************************************************** *
 * ConnectAC: acquires data from socket as it comes.
 * ************************************************************************** */
class ConnectAC {
public: 
    typedef enum {connAlive = 0, connEOF, connException} ConnectState;

    ConnectAC(Socket* use_sock) : _socket(use_sock), _state(connAlive) {}
    virtual ~ConnectAC() { delete _socket; }

    ConnectState State(void) const { return _state; };
    unsigned     getId(void)  const { return (unsigned)(_socket->getSocket()); }
    Socket *     getSocket(void) const { return _socket; }

    virtual void Close(bool abort = false)
    {
        if (abort)
            _socket->Abort();
        else
            _socket->Close();
    }

    virtual ConnectState onReadEvent(void) = 0;
    virtual ConnectState onErrorEvent(void) = 0;

protected:
    ConnectState  _state;
    Socket *      _socket;
};


/* ************************************************************************** *
 * SocketAcquirerAC: accumulates data from socket until required number of
 * bytes is red. It's helpfull to use in ConnectAC::onReadEvent()
 * ************************************************************************** */
class SocketAcquirerAC {
public:
    typedef enum {
        acqSockErr = -3, acqDataErr = -2, acqEOF = -1, acqAwaits = 0, acqComplete = 1
    } SAcqStatus;

protected:
    uint8_t *       dbuf;
    unsigned int    numRed;
    unsigned int    num2Read;
    SAcqStatus      status;

public:
    SocketAcquirerAC(uint8_t * use_buf, unsigned int min_read)
        : status(acqAwaits), dbuf(use_buf), numRed(0), num2Read(min_read)
    { }

    void Reset(uint8_t * use_buf, unsigned int min_read)
    {
        status = acqAwaits; dbuf = use_buf; numRed = 0; num2Read = min_read;
    }
    //Read next portion of data from socket
    SAcqStatus readSocket(Socket * use_sock)
    {
        int n = use_sock->Read((char*)dbuf + numRed, num2Read - numRed);
        if (!n)
            return status = SocketAcquirerAC::acqEOF;
        if (n < 0)
           return  status = ((errno != EINTR) ?
                             onSocketError() : SocketAcquirerAC::acqAwaits);
        numRed += n;
        return status = ((numRed == num2Read) ?
                            onBytesReceived((unsigned)n) : SocketAcquirerAC::acqAwaits);
    }

    SAcqStatus Status(void) const  { return status; }

    // -- SocketAcquirerAC interface methods
    virtual SAcqStatus onBytesReceived(unsigned last_red) = 0;
    virtual SAcqStatus onSocketError(void) = 0;
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_CONNECT_DEFS_HPP */

