#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include <assert.h>

#include "util/BinDump.hpp"
using smsc::util::DumpHex;
//using smsc::util::DumpDbg;

#include "inman/interaction/connect.hpp"
using smsc::inman::interaction::ObjectBuffer;

using namespace smsc::util;

namespace smsc  {
namespace inman {
namespace interaction {

extern const ConnectParms _ConnectParms_DFLT = {
    /*bufSndSz = */1024, /*bufRcvSz = */1028, /*maxPckSz = */32767
};

/* ************************************************************************** *
 * classes PckAcquirer implementation:
 * ************************************************************************** */
SocketAcquirerAC::SAcqStatus PckAcquirer::onSocketError(void)
{
    exc.reset(new CustomException("%s: socket error, code %u: %s", logId, errno,
                                    errno ? strerror(errno) : ""));
    return SocketAcquirerAC::acqSockErr;
}

SocketAcquirerAC::SAcqStatus PckAcquirer::onBytesReceived(unsigned lastRed)
{
    smsc_log_debug(logger, "%s: received %u of %ub: %s%s", logId, numRed, num2Read,
                   "0x", DumpHex(numRed, dbuf).c_str());
//                  "\n", DumpDbg(numRed, dbuf).c_str());
    if (_state == pckIdle) {  //packet length is just red
        uint32_t oct2read = ntohl(*(uint32_t*)len.buf);
        len.ui = oct2read;
        if (oct2read > maxPckSz) {
            exc.reset(new CustomException("%s: incoming packet is too large: %ub",
                                          logId, oct2read));
            return SocketAcquirerAC::acqDataErr;
        }
        if (objBuf.get())
            objBuf->reset(oct2read);
        else
            objBuf.reset(new ObjectBuffer(oct2read));
        dbuf = objBuf->get();
        numRed = 0;
        num2Read = oct2read;
        _state = pckLength;
        return SocketAcquirerAC::acqAwaits;
    }
    assert((_state == pckLength));
    //rest of packet was red, finilize objBuf, (_state == pckLength)
    objBuf->setDataSize(len.ui);
    _state = pckBody;
    return SocketAcquirerAC::acqComplete;
}

/* ************************************************************************** *
 * class Connect implementation:
 * ************************************************************************** */
Connect::Connect(Socket* sock, SerializerITF * serializer, Logger * uselog/* = NULL*/,
                 ConnectParms * use_prm/* = NULL*/)
    : ConnectAC(sock), _objSerializer(serializer), _exc(NULL), logger(uselog)
{
    assert(_socket && serializer);
    if (!uselog)
        logger = Logger::getInstance("smsc.inman.Connect");
    _parms = use_prm ? *use_prm : _ConnectParms_DFLT;
    snprintf(_logId, sizeof(_logId)-1, "Connect[%u]", (unsigned)_socket->getSocket());
    pckAcq.Init(_parms.maxPckSz, _logId, logger);
}

/* -------------------------------------------------------------------------- *
 * ConnectAC interface implementation:
 * -------------------------------------------------------------------------- */
ConnectAC::ConnectState Connect::onReadEvent(void)
{
    switch (pckAcq.readSocket(_socket)) {
    case PckAcquirer::acqEOF: {
        smsc_log_debug(logger, "%s: remote point closed socket", _logId);
        _state = ConnectAC::connEOF;
    } break;

    case PckAcquirer::acqComplete: {
        std::auto_ptr<SerializablePacketAC> msg;
        try { msg.reset(_objSerializer->deserialize(pckAcq.objBuf));
        } catch (SerializerException & exc) {
            _exc.reset(new CustomException("%s: %s", _logId, exc.what()));
        }
        if (!_exc.get()) {
            notifyByMsg(msg); //may set the _exc
            pckAcq.Reset();   //acqAwaits
        }
        if (_exc.get())
            notifyByExc();
    } break;

    case PckAcquirer::acqDataErr: case PckAcquirer::acqSockErr: {
        _exc.reset(pckAcq.exc.release());
        notifyByExc();
    } break;

    default:; // case PckAcquirer::acqAwaits:
    } /* eosw */
    return _state;
}

//passes Connect exception to connect listeners
ConnectAC::ConnectState Connect::onErrorEvent(bool abort/* = false*/)
{
    if (abort)
        _exc.reset(new CustomException("%s: socket is to be aborted", _logId));
    else
        _exc.reset(new CustomException("%s: socket error, code %u: %s", _logId, errno,
                                    errno ? strerror(errno) : ""));
    notifyByExc();
    //unconditionally close connection
    return _state = ConnectAC::connException;
}

/* -------------------------------------------------------------------------- *
 * Public methods:
 * -------------------------------------------------------------------------- */
//sends bytes directly to socket, 
//returns -1 on error, otherwise - number of bytes sent
int  Connect::send(const unsigned char *buf, int bufSz)
{
    MutexGuard  tmp(sndSync);
    int n = _socket->Write((const char *)buf, bufSz);
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

//serializes and sends object to socket in length prefixed format,
//returns -1 on error, or number of total bytes sent
int  Connect::sendPck(SerializablePacketAC* pck)
{
    ObjectBuffer    buffer(_parms.bufSndSz);
    buffer.setPos(4);
    pck->serialize(buffer);
    {
        uint32_t len = htonl((uint32_t)buffer.getPos() - 4);
        memcpy(buffer.get(), (const void *)&len, 4);
    }
    return send(buffer.get(), buffer.getPos());
}

/* -------------------------------------------------------------------------- *
 * Private/Protected methods:
 * -------------------------------------------------------------------------- */
void Connect::notifyByMsg(std::auto_ptr<SerializablePacketAC>& p_msg)
{
    GRDNode *it = begin();
    for (; it && p_msg.get(); it = next(it)) {
        try { it->val->onPacketReceived(this, p_msg);
        } catch (std::exception& lexc) {
            _exc.reset(new CustomException("%s: %s", _logId, lexc.what()));
            smsc_log_error(logger, "%s", _exc->what());
        }
    }
    if (it)
        it->unmark();
}

void Connect::notifyByExc(void)
{
    smsc_log_error(logger, _exc->what());
    GRDNode *it = begin();
    for (; it && _exc.get(); it = next(it)) {
        try { it->val->onConnectError(this, _exc);
        } catch (std::exception& lexc) {
            smsc_log_error(logger, "%s: %s", _logId, lexc.what());
        }
    }
    if (it)
        it->unmark();
    if (_exc.get()) //Listeners do not reset connect exception!!!
        _state = ConnectAC::connException;
}
} // namespace interaction
} // namespace inman
} // namespace smsc

