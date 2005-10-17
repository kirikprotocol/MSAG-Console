static char const ident[] = "$Id$";
#include <assert.h>
#include <stdexcept>

#include "serializer.hpp"
#include "messages.hpp"

using std::runtime_error;

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
        , logger( Logger::getInstance("smsc.inman.interaction.ObjectPipe") )
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
        , logger( Logger::getInstance("smsc.inman.interaction.ObjectPipe") )
{
    assert(serializer);
    assert( socket );
}

ObjectPipe::~ObjectPipe()
{
}


#define socketRead(n, buf, readMax, readMin) \
    if ((n = socket->Read(buf, (readMax))) < (readMin)) { \
        smsc_log_error( logger, "Can't read from socket" ); return NULL; }

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
    smsc_log_debug(logger, "Recv: %s", dump(n, (unsigned char*)buf).c_str(), true);
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
    smsc_log_debug(logger, "Send: %s", dump( buffer.GetPos(), (unsigned char*)buffer.get() ).c_str(), true );
}

} // namespace inap
} // namespace inmgr
} // namespace smsc
