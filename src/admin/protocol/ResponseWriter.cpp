#include "ResponseWriter.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>

namespace smsc {
namespace admin {
namespace protocol {

ResponseWriter::ResponseWriter(int admSocket)
{
	sock = admSocket;
}

ResponseWriter::~ResponseWriter()
{
	sock = 0;
}

void ResponseWriter::write(Response& response)
	throw (AdminException &)
{
	const char * const message = response.getText();
	size_t length = strlen(message);
	writeLength(length);
	writeBuf(message, length);
}

void ResponseWriter::writeLength(size_t length)
	throw (AdminException &)
{
	uint8_t buf[4];
	for (int i=0; i<4; i++)
	{
		buf[3-i] = length & 0xFF;
		length >>= 8;
	}
	writeBuf(buf, sizeof(buf));
}

void ResponseWriter::writeBuf(const void * const buf, size_t len)
	throw (AdminException &)
{
	for (size_t writed = 0; writed < len; )
	{
		size_t writed_now = send(sock, buf, len - writed, 0);
		if (writed_now == -1)
		{
			throw AdminException("Write Error occured");
		}
		writed += writed_now;
	}
}

}
}
}

