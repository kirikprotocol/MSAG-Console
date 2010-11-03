#ifndef __SMSC_INMAN_CONNECT_DEFS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_CONNECT_DEFS_HPP

#include "core/network/Socket.hpp"

namespace smsc  {
namespace inman {
namespace interaction  {

using smsc::core::network::Socket;

/* ************************************************************************** *
 * ConnectAC: acquires data from socket as it comes.
 * ************************************************************************** */
class ConnectAC {
public: 
    enum ConnectState {connAlive = 0, connEOF, connException};

protected:
    ConnectState  _state;
    Socket *      _socket;

public: 
    ConnectAC(Socket * use_sock)
        : _state(connAlive), _socket(use_sock)
    { }
    virtual ~ConnectAC()
    {
        Close(); delete _socket;
    }

    ConnectState State(void) const { return _state; };
    unsigned     getId(void)  const { return (unsigned)(_socket->getSocket()); }
    Socket *     getSocket(void) const { return _socket; }

    virtual void Close(bool abort = false)
    {
        if (abort)
            _socket->Abort();
        else {
            _socket->Close();
            if (_socket->getSocket() != INVALID_SOCKET)
                _socket->Abort();
        }
        _state = connEOF;
    }

    //called if EOF notification or data for reading arrived on socket,
    //connect may be closed depending on returned status.
    virtual ConnectState onReadEvent(void) = 0;
    //called if error condition is pending on socket or socket is to be
    //abnormally closed (aborted).
    virtual ConnectState onErrorEvent(bool abort = false) = 0;
};


/* ************************************************************************** *
 * SocketAcquirerAC: accumulates data from socket until required number of
 * bytes is red. It's helpfull to use in ConnectAC::onReadEvent()
 * ************************************************************************** */
class SocketAcquirerAC {
public:
    enum SAcqStatus {
        acqSockErr = -3, acqDataErr = -2, acqEOF = -1, acqAwaits = 0, acqComplete = 1
    };

protected:
    SAcqStatus      status;
    uint8_t *       dbuf;
    unsigned int    numRed;
    unsigned int    num2Read;

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

