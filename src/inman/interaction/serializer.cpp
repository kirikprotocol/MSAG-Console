static char const ident[] = "$Id$";
#include <assert.h>
#include <stdexcept>

#include "inman/interaction/messages.hpp"
#include "inman/common/util.hpp"

using std::runtime_error;
using smsc::inman::common::dump;

namespace smsc  {
namespace inman {
namespace interaction  {

/* ************************************************************************** *
 * class ObjectPipe implementation:
 * ************************************************************************** */
ObjectPipe::ObjectPipe(Socket* sock, SerializerITF * serializer, unsigned pipe_format)
        : socket( sock )
        , _objSerializer(serializer)
        , _format(pipe_format)
        , logger( Logger::getInstance("smsc.inman.ObjectPipe") )
{
    if (_format > PipeFormat::lengthPrefixed)
        _format = PipeFormat::straightData;
    assert(serializer);
    assert( socket );
}

ObjectPipe::ObjectPipe(Socket* sock, SerializerITF * serializer)
        : socket( sock )
        , _objSerializer(serializer)
        , _format(PipeFormat::straightData)
        , logger( Logger::getInstance("smsc.inman.ObjectPipe") )
{
    assert(serializer);
    assert( socket );
}

ObjectPipe::~ObjectPipe()
{
}

void ObjectPipe::setLogger(Logger * newlog)
{
    logger = newlog;
}

#define socketRead(n, buf, readMax, readMin) \
    if ((n = socket->Read(buf, (readMax))) < (readMin)) { \
        if (!n) smsc_log_debug(logger, "Pipe[%X]: socket empty!", (unsigned int)socket->getSocket()); \
        else    smsc_log_error(logger, "Pipe[%X]: error reading from socket! [%d of %d]", \
                            (unsigned int)socket->getSocket(), n, (int)readMin ); \
        return NULL; \
    }

SerializableObject* ObjectPipe::receive()
{
    int             n;
    char            buf[1024];
    uint32_t        oct2read = sizeof(buf);
    ObjectBuffer    buffer(sizeof(buf));

    if (_format == PipeFormat::lengthPrefixed) {
        socketRead(n, buf, sizeof(uint32_t), sizeof(uint32_t));
        oct2read = ntohl((uint32_t)n);

        int     num = 0;
        while (oct2read -= num) {
            num = ((oct2read) < (sizeof(buf)) ? (oct2read) : (sizeof(buf)));
            socketRead(n, buf, num, num);
            buffer.Append(buf, n);
        }
    } else { //PipeFormat::straightData
        socketRead(n, buf, sizeof(buf) - 1, 1);
        buffer.Append(buf, n);
    }
    buffer.SetPos(0);
    smsc_log_debug(logger, "Pipe[%X]: read %db: %s", (unsigned int)socket->getSocket(),
                   n, dump(n, (unsigned char*)buf, false).c_str());
    return _objSerializer->deserialize(buffer);
}
#undef socketRead

void ObjectPipe::send(SerializableObject* obj)
{
    assert( obj );
    ObjectBuffer buffer(16);
    _objSerializer->serialize(obj, buffer);

    if (_format == PipeFormat::lengthPrefixed) {
        uint32_t len = htonl((uint32_t)buffer.GetPos());
        socket->Write((const char *)&len, sizeof(uint32_t));
    }
    socket->Write(buffer.get(), buffer.GetPos());
    smsc_log_debug(logger, "Pipe[%X]: send %db: %s", (unsigned int)socket->getSocket(),
                   buffer.GetPos(), dump(buffer.GetPos(),
                                         (unsigned char*)buffer.get(), false).c_str());
}

} // namespace inap
} // namespace inmgr
} // namespace smsc
