/* ************************************************************************** *
 * Acquisition Connect: accumulates data from socket until whole length
 * prefixed packet is get, deserializes it and then passes to listeners.
 * Requires external module (like smsc::inman::interaction::Server) that
 * checks socket for incoming data and calls Connect methods.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_ACQ_CONNECT_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_ACQ_CONNECT_HPP

#include "logger/Logger.h"
#include "inman/common/Observatory.hpp"
#include "inman/interaction/serializer.hpp"
#include "inman/interaction/ConnectDefs.hpp"

namespace smsc  {
namespace inman {
namespace interaction {

using smsc::logger::Logger;
using smsc::inman::interaction::SerializablePacketAC;
using smsc::util::GRDObservatoryOfT;


/* ************************************************************************** *
 * PckAcquirer: accumulates data from socket until whole length prefixed
 * packet is red.
 * ************************************************************************** */
class PckAcquirer : public SocketAcquirerAC {
public:
    enum PckState { pckIdle = 0, pckLength, pckBody };

    std::auto_ptr<ObjectBuffer>     objBuf;
    std::auto_ptr<smsc::util::CustomException>  exc;

private:
    using SocketAcquirerAC::Reset; //hide it to avoid annoying CC warnings

protected:
    union { //force correct alignment of len.buf on 64-bit platforms
        uint32_t    ui;
        uint8_t     buf[sizeof(uint32_t)];
    } len;
    uint32_t    maxPckSz;
    PckState    _state;
    const char *logId;
    Logger *    logger;

public:
    PckAcquirer()
        : SocketAcquirerAC(len.buf, 4), maxPckSz(1024), _state(pckIdle)
        , logId(""), logger(NULL)
    { }

    PckAcquirer(uint32_t max_pckSize, const char * log_id, Logger * use_log)
        : SocketAcquirerAC(len.buf, 4), maxPckSz(max_pckSize), _state(pckIdle)
        , logId(log_id), logger(use_log)
    { }

    void Init(uint32_t max_pckSize, const char * log_id, Logger * use_log)
    {
        maxPckSz = max_pckSize; logId = log_id; logger = use_log;
    }

    void Reset(void)
    { 
        SocketAcquirerAC::Reset(len.buf, 4);
        _state = pckIdle; exc.reset(NULL);
        if (objBuf.get())
            objBuf->reset();
    }

    // -- SocketAcquirerAC interface methods inmplemenation:
    SocketAcquirerAC::SAcqStatus onBytesReceived(unsigned last_red);
    SocketAcquirerAC::SAcqStatus onSocketError(void);
};

/* ************************************************************************** *
 * class Connect:
 * ************************************************************************** */
typedef struct {
    unsigned int  bufSndSz; //default buffer size for sending packet
    unsigned int  bufRcvSz; //default buffer size for receiving packet
    unsigned int  maxPckSz; //maximum allowed packet size = 32767
} ConnectParms;

extern const ConnectParms _ConnectParms_DFLT;

class Connect;

//NOTE: it's assumed that if Connect has exception set upon return from these callbacks,
//the controlling TCP server destroys this Connect.
class ConnectListenerITF {
protected:
    virtual ~ConnectListenerITF() //forbid interface destruction
    { }

public:
    //NOTE: if listener takes ownership of packet, remaining listeners will not be notified.
    virtual void onPacketReceived(Connect* conn, std::auto_ptr<SerializablePacketAC>& recv_cmd)
                     /*throw(std::exception) */= 0;
    //NOTE: it's recommended to reset exception if it doesn't prevent entire Connect to function
    virtual void onConnectError(Connect* conn, std::auto_ptr<smsc::util::CustomException>& p_exc) = 0;
};


//NOTE: Connect methods, in case of error, create SystemError or SerializerException
//(returned by hasException(), but instead of throwing it, passes latter to listeners.
//Listeners are expected to either reset exception in order to continue Connect
//processing or may leave exception forcing the Connect destruction by controlling
//TCP server.
class Connect : public ConnectAC, public GRDObservatoryOfT<ConnectListenerITF> {
public:
    Connect(Socket* sock, SerializerITF * serializer, Logger * uselog = NULL,
            ConnectParms * prm = NULL);
    virtual ~Connect() { }

    void Reset(void)
    {
        pckAcq.Reset(); _exc.reset();
        snprintf(_logId, sizeof(_logId)-1, "Connect[%u]", (unsigned)_socket->getSocket());
        _state = ConnectAC::connAlive;
    }
    // -- ConnectAC iterface methods:
    //Accumulates packets from socket and passes them to listeners,
    //called if EOF notification or data for reading arrived on socket,
    //connect may be closed depending on returned status.
    ConnectAC::ConnectState onReadEvent(void);
    //Notifies connect listeners about error,
    //called if error condition is pending on socket or socket is to be
    //abnormally closed (aborted).
    ConnectAC::ConnectState onErrorEvent(bool abort = false);

    //sends bytes directly to socket, 
    //returns -1 on error, otherwise - number of bytes sent
    int     send(const unsigned char *buf, int bufSz);
    //serializes and sends packet to socket in length prefixed format,
    //returns -1 on error, or number of total bytes sent
    int     sendPck(SerializablePacketAC* pck);

    //returns last connect exception
    smsc::util::CustomException* hasException(void) const { return _exc.get(); }
    void             resetException(void) { _exc.reset(NULL); }

protected:
    void notifyByExc(void);
    void notifyByMsg(std::auto_ptr<SerializablePacketAC>& p_msg);

    smsc::core::synchronization::Mutex  sndSync;
    ConnectParms    _parms;
    SerializerITF * _objSerializer;
    PckAcquirer     pckAcq;
    std::auto_ptr<smsc::util::CustomException> _exc;   //last connect exception
    char            _logId[sizeof("Connect[%u]") + sizeof(unsigned)*3 + 1];
    Logger*         logger;
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_ACQ_CONNECT_HPP */

