#ifndef SMSC_ADMIN_PROTOCOL_RESPONSE_WRITER
#define SMSC_ADMIN_PROTOCOL_RESPONSE_WRITER

#include <admin/AdminException.h>
#include <admin/protocol/Response.h>
#include <stddef.h>

namespace smsc {
namespace admin {
namespace protocol {

class ResponseWriter
{
public:
	ResponseWriter(int admSocket);
	~ResponseWriter();
	void write(Response& response) throw (AdminException &);

protected:
	void writeBuf(const void * const buf, size_t len) throw (AdminException &);
	void writeLength(size_t length) throw (AdminException &);
	int sock;

private:
};

}
}
}
#endif // ifndef SMSC_ADMIN_PROTOCOL_RESPONSE_WRITER
