#ifndef SMSC_ADMIN_PROTOCOL_COMMAND_READER
#define SMSC_ADMIN_PROTOCOL_COMMAND_READER

#include <xercesc/dom/DOM.hpp>

#include <admin/protocol/Command.h>
#include <admin/AdminException.h>
#include <core/network/Socket.hpp>

namespace smsc {
namespace admin {
namespace protocol {

using namespace xercesc;
using smsc::core::network::Socket;

class CommandReader
{
public:
  CommandReader(Socket * admSocket);
  virtual ~CommandReader();
  Command * read() throw (AdminException);
  bool canRead() throw (AdminException)
  {
    switch (sock->canRead())
    {
    case 1:
      return true;
    case 0:
      return false;
    case -1:
      throw AdminException("Socket disconnected");
    default:
      throw AdminException("Unknown error on CommandReader.canRead()");
    }
  }

protected:
  char * getCommandName(const DOMDocument *data);
  void readMessageBody(XMLByte * buf, uint32_t len) throw (AdminException);
  uint32_t readMessageLength() throw (AdminException);
  virtual int getCommandIdByName(const char * const command_name);
  
  smsc::logger::Logger *logger;
  Socket * sock;

  virtual Command * createCommand(int id, const DOMDocument *data);
  Command * parseCommand(DOMInputSource &source) throw (AdminException);

private:
};

}
}
}

#endif // ifndef SMSC_ADMIN_PROTOCOL_COMMAND_READER
