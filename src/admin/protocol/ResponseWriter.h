#ifndef SMSC_ADMIN_PROTOCOL_RESPONSE_WRITER
#define SMSC_ADMIN_PROTOCOL_RESPONSE_WRITER

#include <admin/AdminException.h>
#include <admin/protocol/Response.h>
#include <core/network/Socket.hpp>
#include <stddef.h>

namespace smsc {
namespace admin {
namespace protocol {

using smsc::core::network::Socket;

class ResponseWriter
{
public:
	ResponseWriter(Socket * admSocket);
	~ResponseWriter();
	void write(Response& response) throw (AdminException);

protected:
	void writeBuf(const void * const buf, size_t len) throw (AdminException);
	void writeLength(size_t length) throw (AdminException);
	Socket * sock;

private:
};

}
}
}
#endif // ifndef SMSC_ADMIN_PROTOCOL_RESPONSE_WRITER
