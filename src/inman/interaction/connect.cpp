static char const ident[] = "$Id$";
#include <assert.h>

#include "inman/interaction/connect.hpp"

using smsc::inman::interaction::ObjectBuffer;
using smsc::inman::common::dump;
using smsc::inman::common::format;

namespace smsc  {
namespace inman {
namespace interaction {

extern const ConnectParms _ConnectParms_DFLT = {
    /*bufSndSz = */1024, /*bufRcvSz = */1028, /*maxPckSz = */32767
};

Connect::Connect(Socket* sock, SerializerITF * serializer,
                 ConnectFormat frm/* = Connect::frmLengthPrefixed*/,
                 Logger * uselog/* = NULL*/)
    : logger(uselog), socket(sock), _frm(frm), _exc(NULL)
    , _objSerializer(serializer), _parms(_ConnectParms_DFLT)
{
    assert(socket && _objSerializer);
    if (!uselog)
        logger = Logger::getInstance("smsc.inman.Connect");
}

Connect::~Connect()
{
    delete socket;
    if (_exc)
        delete _exc;
}

void Connect::setConnectFormat(Connect::ConnectFormat frm, ConnectParms * prm/* = NULL*/)
{
    _frm = frm;
    if (prm)
        _parms = *prm;
}

void Connect::resetException(void)
{
    if (_exc) {
        delete _exc;
        _exc = NULL;
    }
}

void Connect::setException(const char * msg, int err_code/* = 0*/)
{
    resetException();
    _exc = new SystemError(msg, err_code);
}

//passes Connect exception to connect listeners
void Connect::handleConnectError(bool fatal/* = false*/)
{
    //this Connect may be desroyed by one of onConnectError() so iterate over copy of list
    ListenerList    cplist = listeners;
    for (ListenerList::iterator it = cplist.begin(); it != cplist.end(); it++) {
        ConnectListener* ptr = *it;
        ptr->onConnectError(this, fatal);
    }
}

//sends bytes directly to socket, 
//returns -1 on error, otherwise - number of bytes sent
int  Connect::send(const unsigned char *buf, int bufSz)
{
    Logger::LogLevel    errLvl = Logger::LEVEL_DEBUG;
    std::string         dstr;

    int n = socket->Write((const char *)buf, bufSz);

    format(dstr, "Connect[%u]: sent %d of %db: ", (unsigned int)
            socket->getSocket(), n, bufSz);
    if (n > 0)
        dump(dstr, (unsigned short)n, (unsigned char*)buf, false);
    if (n < bufSz) {
        errLvl = Logger::LEVEL_ERROR;
        setException(dstr.c_str(), errno);
    }
    logger->log(errLvl, dstr.c_str());

    return (n < bufSz) ? (-1) : n;
}

//serializes and sends object to socket according to ConnectFormat
//returns -1 on error, or number of total bytes sent
int  Connect::sendObj(SerializableObject* obj)
{
//    assert(obj);
    int             n;
    ObjectBuffer    buffer(_parms.bufSndSz);

    _objSerializer->serialize(obj, buffer);

    if (_frm == Connect::frmLengthPrefixed) {
        uint32_t len = htonl((uint32_t)buffer.getPos());

        int n = send((const unsigned char *)&len, sizeof(uint32_t));
        if (n < 0)
            return n;
    }
    return send(buffer.get(), buffer.getPos());
}

//receives bytes from socket,
//returns -1 on error, otherwise - number of bytes red
//NOTE: minToRead should be <= bufSz
int  Connect::receive(const unsigned char *buf, int bufSz, int minToRead)
{
    Logger::LogLevel    errLvl = Logger::LEVEL_DEBUG;
    std::string         dstr;

    int n = socket->Read((char*)buf, bufSz);
    if (!n) {
        smsc_log_debug(logger, "Connect[%u]: socket empty!", (unsigned int)socket->getSocket());
        return n;
    }
    format(dstr, "Connect[%u]: %sreceived %d of %db: ", (unsigned int)socket->getSocket(),
            (n < minToRead) ? "error on socket, " : "", n, minToRead);
    if (n > 0)
        dump(dstr, (unsigned short)n, (unsigned char*)buf, false);
    if (n < minToRead) {
        errLvl = Logger::LEVEL_ERROR;
        setException(dstr.c_str(), errno);
    }
    logger->log(errLvl, dstr.c_str());
    return (n < minToRead) ? (-1) : n;
}

//receives and deserializes object from socket,
//return NULL on error, otherwise - allocated object
SerializableObject* Connect::receiveObj(void)
{
    int             n;
    uint32_t        oct2read = _parms.bufRcvSz;
    ObjectBuffer *  buffer = NULL;
    
    if (_frm == Connect::frmLengthPrefixed) {
        unsigned char  lenbuf[sizeof(uint32_t) + 2];
        if ((n = receive(lenbuf, sizeof(uint32_t), sizeof(uint32_t))) <= 0)
            return NULL;
        oct2read = ntohl(*(uint32_t*)lenbuf);

        if (oct2read > _parms.maxPckSz) {
            std::string dstr = format("Connect[%u]: incoming packet is too large: %ub",
                                        (unsigned int)socket->getSocket(), oct2read);
            setException(dstr.c_str(), 0);
            return NULL;
        }
    }
    buffer = new ObjectBuffer(oct2read);

    if ((n = receive(buffer->get(), oct2read,
                     (_frm == Connect::frmLengthPrefixed) ? oct2read : 1)) <= 0)
        return NULL;
    buffer->setDataSize(n);

    SerializableObject* obj = NULL;
    try { obj = _objSerializer->deserializeAndOwn(buffer, true); }
    catch (SerializerException & exc) {
        setException(format("Connect[%u]: %s", (unsigned int)socket->getSocket(),
                            exc.what()).c_str(), 0);
    }
    return obj;
}

//listens for input objects and passes it to connect listeners
//Returns false if no data was red from socket
bool Connect::process(void)
{
    SerializableObject* cmd = receiveObj();
	
    if (!cmd && !_exc)
	return false;

    bool res = cmd || _exc;
    for (ListenerList::iterator it = listeners.begin(); it != listeners.end(); it++) {
        ConnectListener* ptr = *it;
        if (_exc)
            ptr->onConnectError(this, false);
        else
            ptr->onCommandReceived(this, cmd);
    }
    resetException();
    if (cmd)
        delete cmd;
    return res;
}

} // namespace interaction
} // namespace inman
} // namespace smsc

