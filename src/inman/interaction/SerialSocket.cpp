#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include <assert.h>

#include "util/BinDump.hpp"
using smsc::util::DumpHex;

#include "inman/interaction/SerialSocket.hpp"
using smsc::inman::interaction::ObjectBuffer;
using smsc::core::synchronization::MutexGuard;

using namespace smsc::util;

namespace smsc  {
namespace inman {
namespace interaction {

extern const SocketParms _SocketParms_DFLT = {
    /*bufSndSz = */1024, /*bufRcvSz = */1028, /*maxPckSz = */32767
};

SerialSocket::SerialSocket(Socket* sock, ConnectFormat frm/* = SerialSocket::frmLengthPrefixed*/,
                 SerializerITF * serializer/* = NULL*/, Logger * uselog/* = NULL*/)
    : socket(sock), _frm(frm), _parms(_SocketParms_DFLT)
    , _objSerializer(serializer), _exc(NULL), logger(uselog)
{
    assert(socket);
    if (!uselog)
        logger = Logger::getInstance("smsc.inman.Connect");
    snprintf(_logId, sizeof(_logId)-1, "Connect[%u]", (unsigned)socket->getSocket());
}

SerialSocket::~SerialSocket()
{
    delete socket;
}

void SerialSocket::close(bool abort/* = false*/)
{
    if (abort)
        socket->Abort();
    else
        socket->Close();
}

void SerialSocket::Init(SerialSocket::ConnectFormat frm, 
                    SerializerITF * serializer, SocketParms * prm/* = NULL*/)
{
    _frm = frm;
    _objSerializer = serializer;
    if (prm)
        _parms = *prm;
}

//sends bytes directly to socket, 
//returns -1 on error, otherwise - number of bytes sent
int  SerialSocket::send(const unsigned char *buf, int bufSz)
{
    MutexGuard  tmp(sndSync);
    int n = socket->Write((const char *)buf, bufSz);
    if ((n < bufSz) || logger->isDebugEnabled()) {
        std::string         dstr;
        Logger::LogLevel    errLvl;

        format(dstr, "%s: sent %d of %db: ", _logId, n, bufSz);
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
int  SerialSocket::sendPck(SerializablePacketAC* pck)
{
    int             offs = 4;
    ObjectBuffer    buffer(_parms.bufSndSz);

    buffer.setPos(offs);
    pck->serialize(buffer);
    if (_frm == SerialSocket::frmLengthPrefixed) {
        uint32_t len = htonl((uint32_t)buffer.getPos() - 4);
        memcpy(buffer.get(), (const void *)&len, 4);
        offs = 0;
    }
    return send(buffer.get() + offs, buffer.getPos());
}

//Thread safe version of receive_buf
int  SerialSocket::receive(unsigned char *buf, int bufSz, int minToRead)
{
    MutexGuard  tmp(rcvSync);
    return receive_buf(buf, bufSz, minToRead);
}

//receives and deserializes object from socket,
//return NULL on error, otherwise - allocated object
SerializablePacketAC* SerialSocket::recvPck(void)
{
    MutexGuard  tmp(rcvSync);
    int             n;
    uint32_t        oct2read = _parms.bufRcvSz;
    
    if (_frm == SerialSocket::frmLengthPrefixed) {
        union { //force correct alignment of len.buf
            uint32_t    ui;
            uint8_t     buf[sizeof(uint32_t)];
        } len;
        if ((n = receive_buf(len.buf, (int)sizeof(uint32_t), (int)sizeof(uint32_t))) <= 0)
            return NULL;
        oct2read = ntohl(*(uint32_t*)len.buf);

        if (oct2read > _parms.maxPckSz) {
            _exc.reset(new CustomException("%s: incoming packet is too large: %ub",
                                            _logId, oct2read));
            return NULL;
        }
    }
    std::auto_ptr<ObjectBuffer> buffer(new ObjectBuffer(oct2read));

    if ((n = receive_buf(buffer->get(), oct2read,
                (_frm == SerialSocket::frmLengthPrefixed) ? oct2read : 1)) <= 0)
        return NULL;
    buffer->setDataSize(n);

    SerializablePacketAC* obj = NULL;
    if (!_objSerializer) {
        _exc.reset(new CustomException("%s: Serializer is not set!", _logId));
    } else {
        try { obj = _objSerializer->deserialize(buffer); 
        } catch (SerializerException & exc) {
            _exc.reset(new CustomException("%s: %s", _logId, exc.what()));
        }
    }
    return obj;
}

/* -------------------------------------------------------------------------- *
 * Private/Protected methods:
 * -------------------------------------------------------------------------- */
//receives bytes from socket,
//returns -1 on error, otherwise - number of bytes red
//NOTE: minToRead should be <= bufSz
int  SerialSocket::receive_buf(unsigned char *buf, int bufSz, int minToRead)
{
    int n = socket->Read((char*)buf, bufSz);
    if (!n) {
        smsc_log_debug(logger, "%s: socket empty!", _logId);
        return n;
    }
    if ((n < minToRead) || logger->isDebugEnabled()) {
        std::string         dstr;
        Logger::LogLevel    errLvl;
        
        format(dstr, "%s: %sreceived %d of %db: ", _logId,
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

