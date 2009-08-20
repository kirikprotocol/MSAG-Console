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

ResponseWriter::ResponseWriter(Socket * admSocket)
{
  sock = admSocket;
}

ResponseWriter::~ResponseWriter()
{
  sock = 0;
}

void ResponseWriter::write(Response& response)throw (AdminException)
{
  const char * const message = response.getText();
  size_t length = strlen(message);
  writeLength(length);
  writeBuf(message, length);
}

void ResponseWriter::writeLength(size_t length)throw (AdminException)
{
  uint8_t buf[4];
  for (int i=0; i<4; i++)
  {
    buf[3-i] = (uint8_t)(length & 0xFF);
    length >>= 8;
  }
  writeBuf(buf, sizeof(buf));
}

void ResponseWriter::writeBuf(const void * const buf, size_t len)throw (AdminException)
{
  for (long writed = 0; writed < (long)len; )
  {
    int writed_now = sock->Write((const char *)(buf) + writed, (int)(len - writed));
    if (writed_now == 0 || writed_now == -1)
    {
      throw AdminException("Connect broken");
    }
    writed += writed_now;
  }
}

}
}
}

