static char const ident[] = "$Id$";
#include <assert.h>

#include "util/BinDump.hpp"
using smsc::util::DumpHex;

#include "inman/interaction/connect.hpp"
using smsc::inman::interaction::ObjectBuffer;

namespace smsc  {
namespace inman {
namespace interaction {

extern const ConnectParms _ConnectParms_DFLT = {
    /*bufSndSz = */1024, /*bufRcvSz = */1028, /*maxPckSz = */32767
};

Connect::Connect(Socket* sock, ConnectFormat frm/* = Connect::frmLengthPrefixed*/,
                 SerializerITF * serializer/* = NULL*/, Logger * uselog/* = NULL*/)
    : logger(uselog), socket(sock), _frm(frm), _exc(NULL)
    , _objSerializer(serializer), _parms(_ConnectParms_DFLT)
{
    assert(socket);
    if (!uselog)
        logger = Logger::getInstance("smsc.inman.Connect");
}

Connect::~Connect()
{
    delete socket;
}

void Connect::close(bool abort/* = false*/)
{
    if (abort)
        socket->Abort();
    else
        socket->Close();
}

void Connect::Init(Connect::ConnectFormat frm, 
                    SerializerITF * serializer, ConnectParms * prm/* = NULL*/)
{
    _frm = frm;
    _objSerializer = serializer;
    if (prm)
        _parms = *prm;
}

//passes Connect exception to connect listeners
void Connect::handleConnectError(bool fatal/* = false*/)
{
    //this Connect may be desroyed by one of onConnectError() so iterate over copy of list
    ListenerList    cplist = listeners;
    for (ListenerList::iterator it = cplist.begin(); it != cplist.end(); it++) {
        ConnectListenerITF* ptr = *it;
        ptr->onConnectError(this, fatal);
    }
}

//sends bytes directly to socket, 
//returns -1 on error, otherwise - number of bytes sent
int  Connect::send(const unsigned char *buf, int bufSz)
{
    MutexGuard  tmp(sndSync);
    int n = socket->Write((const char *)buf, bufSz);
    if ((n < bufSz) || logger->isDebugEnabled()) {
        std::string         dstr;
        Logger::LogLevel    errLvl;

        format(dstr, "Connect[%u]: sent %d of %db: ", (unsigned int)
                socket->getSocket(), n, bufSz);
        if (n > 0) {
            dstr += "0x"; DumpHex(dstr, (unsigned short)n, (unsigned char*)buf);
        }
        if (n < bufSz) {
            errLvl = Logger::LEVEL_ERROR;
            _exc.reset(new SystemError(dstr.c_str(), errno));
        } else
            errLvl = Logger::LEVEL_DEBUG;

        logger->log(errLvl, dstr.c_str());
    }
    return (n < bufSz) ? (-1) : n;
}

//serializes and sends object to socket according to ConnectFormat
//returns -1 on error, or number of total bytes sent
int  Connect::sendPck(SerializablePacketAC* pck)
{
    int             offs = 4;
    ObjectBuffer    buffer(_parms.bufSndSz);

    buffer.setPos(offs);
    pck->serialize(buffer);
    if (_frm == Connect::frmLengthPrefixed) {
        uint32_t len = htonl((uint32_t)buffer.getPos() - 4);
        memcpy(buffer.get(), (const void *)&len, 4);
        offs = 0;
    }
    return send(buffer.get() + offs, buffer.getPos());
}

//Thread safe version of receive_buf
int  Connect::receive(unsigned char *buf, int bufSz, int minToRead)
{
    MutexGuard  tmp(rcvSync);
    return receive_buf(buf, bufSz, minToRead);
}

//receives and deserializes object from socket,
//return NULL on error, otherwise - allocated object
SerializablePacketAC* Connect::recvPck(void)
{
    MutexGuard  tmp(rcvSync);
    int             n;
    uint32_t        oct2read = _parms.bufRcvSz;
    
    if (_frm == Connect::frmLengthPrefixed) {
        union { //force correct alignment of len.buf
            uint32_t    ui;
            uint8_t     buf[sizeof(uint32_t)];
        } len;
        if ((n = receive_buf(len.buf, sizeof(uint32_t), sizeof(uint32_t))) <= 0)
            return NULL;
        oct2read = ntohl(*(uint32_t*)len.buf);

        if (oct2read > _parms.maxPckSz) {
            std::string dstr;
            format(dstr, "Connect[%u]: incoming packet is too large: %ub",
                                        (unsigned int)socket->getSocket(), oct2read);
            _exc.reset(new CustomException(dstr.c_str()));
            return NULL;
        }
    }
    std::auto_ptr<ObjectBuffer> buffer(new ObjectBuffer(oct2read));

    if ((n = receive_buf(buffer->get(), oct2read,
                (_frm == Connect::frmLengthPrefixed) ? oct2read : 1)) <= 0)
        return NULL;
    buffer->setDataSize(n);

    SerializablePacketAC* obj = NULL;
    if (!_objSerializer) {
        std::string dstr;
        format(dstr, "Connect[%u]: Serializer is not set!", (unsigned int)socket->getSocket());
        _exc.reset(new CustomException(dstr.c_str()));
    } else {
        try { obj = _objSerializer->deserialize(buffer); 
        } catch (SerializerException & exc) {
            std::string dstr;
            format(dstr, "Connect[%u]: %s", (unsigned int)socket->getSocket(), exc.what());
            _exc.reset(new CustomException(dstr.c_str()));
        }
    }
    return obj;
}

//listens for input objects and passes it to connect listeners
//Returns false if no data was red from socket
bool Connect::process(void)
{
    std::auto_ptr<SerializablePacketAC> cmd(recvPck());
	
    if (!cmd.get() && !_exc.get())
	return false;

    bool res = cmd.get() || _exc.get();
    for (ListenerList::iterator it = listeners.begin(); it != listeners.end(); it++) {
        ConnectListenerITF* ptr = *it;
        if (_exc.get())
            ptr->onConnectError(this, false);
        else
            ptr->onCommandReceived(this, cmd);
    }
    _exc.reset(NULL);
    return res;
}

/* -------------------------------------------------------------------------- *
 * Private/Protected methods:
 * -------------------------------------------------------------------------- */
//receives bytes from socket,
//returns -1 on error, otherwise - number of bytes red
//NOTE: minToRead should be <= bufSz
int  Connect::receive_buf(unsigned char *buf, int bufSz, int minToRead)
{
    int n = socket->Read((char*)buf, bufSz);
    if (!n) {
        smsc_log_debug(logger, "Connect[%u]: socket empty!", (unsigned int)socket->getSocket());
        return n;
    }
    if ((n < minToRead) || logger->isDebugEnabled()) {
        std::string         dstr;
        Logger::LogLevel    errLvl;
        
        format(dstr, "Connect[%u]: %sreceived %d of %db: ", (unsigned int)socket->getSocket(),
                (n < minToRead) ? "error, " : "", n, minToRead);
        if (n > 0) {
            dstr += "0x"; DumpHex(dstr, (unsigned short)n, (unsigned char*)buf/*, _HexDump_CVSD*/);
//            dstr += "\n"; DumpDbg(dstr, (unsigned short)n, (unsigned char*)buf);
        }
        if (n < minToRead) {
            errLvl = Logger::LEVEL_ERROR;
            _exc.reset(new SystemError(dstr.c_str(), errno));
        } else
            errLvl = Logger::LEVEL_DEBUG;
        logger->log(errLvl, dstr.c_str());
    }
    return (n < minToRead) ? (-1) : n;
}

} // namespace interaction
} // namespace inman
} // namespace smsc

