#ifndef SMSC_ADMIN_PROTOCOL_COMMAND_READER
#define SMSC_ADMIN_PROTOCOL_COMMAND_READER

#include <admin/protocol/Command.h>
#include <xercesc/sax/InputSource.hpp>
#include <admin/AdminException.h>

namespace smsc {
namespace admin {
namespace protocol {

class CommandReader
{
public:
	CommandReader(int admSocket);
	~CommandReader();
	Command * read() throw (AdminException &);

protected:
	char * getCommandName(DOM_Document data);
	void readMessageBody(XMLByte * buf, uint32_t len) throw (AdminException &);
	uint32_t readMessageLength() throw (AdminException &);
	log4cpp::Category &logger;
	int sock;
	DOMParser * parser;
	
	DOMParser * createParser();
	Command * createCommand(Command::Id id, DOM_Document data);
	Command * parseCommand(InputSource &source) throw (AdminException &);

private:
};

}
}
}

#endif // ifndef SMSC_ADMIN_PROTOCOL_COMMAND_READER

