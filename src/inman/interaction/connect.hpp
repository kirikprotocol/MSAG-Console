#ident "$Id$"

#ifndef __SMSC_INMAN_TCP_CONNECT__
#define __SMSC_INMAN_TCP_CONNECT__

#include "inman/interaction/serializer.hpp"
#include "inman/common/observable.hpp"
#include "inman/common/errors.hpp"
#include "core/network/Socket.hpp"
#include "logger/Logger.h"

using smsc::logger::Logger;
using smsc::core::network::Socket;
using smsc::inman::common::ObservableT;
using smsc::inman::common::SystemError;
using smsc::inman::interaction::SerializableObject;
using smsc::inman::interaction::SerializerITF;

namespace smsc  {
namespace inman {
namespace interaction {

typedef struct {
    unsigned int  bufSndSz; //default buffer size for sending packet
    unsigned int  bufRcvSz; //default buffer size for receiving packet
    unsigned int  maxPckSz; //maximum allowwed packet size = 32767
} ConnectParms;

extern const ConnectParms _ConnectParms_DFLT;

class ConnectListener;
//Connect: sends/receives object to/from TCP socket
//NOTE: Connect methods, in case of error, create SystemError or SerializerException
//(returned by hasException(), but do not throw it.
//It's caller responsibility to manage this exception.
class Connect : public ObservableT< ConnectListener >
{
public:
    typedef enum { frmStraightData = 0, frmLengthPrefixed = 1 } ConnectFormat;

    Connect(Socket* sock, SerializerITF * serializer,
            ConnectFormat frm = Connect::frmLengthPrefixed, Logger * uselog = NULL);
    virtual ~Connect();

    Socket* getSocket() const   { return socket; }
    SOCKET  getSocketId() const { return socket->getSocket(); }
    void    setConnectFormat(Connect::ConnectFormat frm, ConnectParms * prm = NULL);

    //sends bytes directly to socket, 
    //returns -1 on error, otherwise - number of bytes sent
    int     send (const unsigned char *buf, int bufSz);
    //receives bytes from socket,
    //returns -1 on error, otherwise - number of bytes red
    int     receive(const unsigned char *buf, int bufSz, int minToRead);

    //serializes and sends object to socket according to ConnectFormat
    //returns -1 on error, or number of total bytes sent
    int     sendObj(SerializableObject* obj); 
    //receives and deserializes object from socket,
    //return NULL on error, otherwise - allocated object
    SerializableObject* receiveObj(void);

    //returns exception created on connect error
    CustomException* hasException(void) const { return _exc; }
    void             resetException(void);

    //listens for input objects and passes it to connect listeners
    //Returns false if no data was red from socket
    bool    process(void);
    //passes Connect exception to connect listeners
    void    handleConnectError(bool fatal/* = false*/);

protected:
    void setException(const char * msg, int err_code = 0);

    Socket*         socket;
    Logger*         logger;
    ConnectFormat       _frm;
    ConnectParms        _parms;
    SerializerITF *     _objSerializer;
    CustomException*    _exc;   //last connect error
};

class ConnectListener
{
    public:
        virtual void onCommandReceived(Connect* conn, SerializableObject* cmd) = 0;
        //NOTE: it's recommended to reset exception if it doesn't prevent entire Connect to function
        virtual void onConnectError(Connect* conn, bool fatal = false) = 0;
};


} //interaction
} //inman
} //smsc

#endif /* __SMSC_INMAN_TCP_CONNECT__ */

