/* ************************************************************************** *
 * Serializable Connect: directly reads from/sends to a socket serializable
 * objects. Designed mainly for testing purposes.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_SRLZ_CONNECT_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_SRLZ_CONNECT_HPP

#include "logger/Logger.h"
#include "core/network/Socket.hpp"

#include "inman/interaction/serializer.hpp"

namespace smsc  {
namespace inman {
namespace interaction {

using smsc::logger::Logger;
using smsc::core::network::Socket;
using smsc::inman::interaction::SerializablePacketAC;

typedef struct {
    unsigned int  bufSndSz; //default buffer size for sending packet
    unsigned int  bufRcvSz; //default buffer size for receiving packet
    unsigned int  maxPckSz; //maximum allowed packet size = 32767
} SocketParms;

extern const SocketParms _SocketParms_DFLT;


//Connect: sends/receives object to/from TCP socket
//NOTE: SerialSocket methods, in case of error, create SystemError
//or SerializerException (returned by hasException(), but do not throw it.
//It's caller responsibility to manage this exception.
class SerialSocket  {
public:
    enum ConnectFormat { frmStraightData = 0, frmLengthPrefixed = 1 };

    SerialSocket(Socket* sock, ConnectFormat frm = SerialSocket::frmStraightData,
            SerializerITF * serializer = NULL, Logger * uselog = NULL);
    virtual ~SerialSocket();

    void    Init(SerialSocket::ConnectFormat frm,
                 SerializerITF * serializer, SocketParms * prm = NULL);

    Socket* getSocket(void) const   { return socket; }
    SOCKET  getId(void) const { return socket->getSocket(); }

    void    close(bool abort = false);
    //sends bytes directly to socket, 
    //returns -1 on error, otherwise - number of bytes sent
    int     send (const unsigned char *buf, int bufSz);
    //receives bytes from socket,
    //returns -1 on error, otherwise - number of bytes red
    int     receive(unsigned char *buf, int bufSz, int minToRead);

    //serializes and sends packet to socket according to ConnectFormat
    //returns -1 on error, or number of total bytes sent
    int     sendPck(SerializablePacketAC* pck);
    //receives and deserializes packet from socket,
    //return NULL on error, otherwise - allocated object
    SerializablePacketAC* recvPck(void);

    //returns exception created on connect error
    CustomException* hasException(void) const { return _exc.get(); }
    void             resetException(void) { _exc.reset(NULL); }

protected:
    int  receive_buf(unsigned char *buf, int bufSz, int minToRead);

    Mutex           sndSync, rcvSync;
    Socket*         socket;
    ConnectFormat   _frm;
    SocketParms    _parms;
    SerializerITF * _objSerializer;
    std::auto_ptr<CustomException> _exc;   //last connect exception
    char            _logId[sizeof("Connect[%u]") + sizeof(unsigned)*3 + 1];
    Logger*         logger;
};

} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_SRLZ_CONNECT_HPP */

