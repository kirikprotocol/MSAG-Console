#ifndef SMSC_ADMIN_PROTOCOL_COMMAND_ADD_SERVICE
#define SMSC_ADMIN_PROTOCOL_COMMAND_ADD_SERVICE

#include <admin/AdminException.h>
#include <admin/protocol/CommandService.h>
#include <util/xml/utilFunctions.h>
#include <admin/daemon/Service.h>
#include <util/cstrings.h>

namespace smsc {
namespace admin {
namespace protocol {

using smsc::admin::AdminException;
using smsc::util::xml::getNodeAttribute;
using smsc::util::xml::getNodeText;
using smsc::util::decode;

typedef std::pair<int, char *> CmdArgument;

class CommandAddService : public CommandService
{
public:
	CommandAddService(DOM_Document doc)	throw (AdminException)
		: CommandService(add_service, doc)
	{
		logger.debug("Add service command");
		try
		{
			DOM_Element elem = doc.getDocumentElement();
			DOM_NodeList list = elem.getElementsByTagName("service");
			if (list.getLength() > 0)
			{
				std::vector<CmdArgument> cmd_args;
				DOM_Node serviceNode = list.item(0);
				DOM_Element &serviceElem = (DOM_Element&) serviceNode;
				serviceName = decode(serviceElem.getAttribute("name"));
				cmdLine = decode(serviceElem.getAttribute("cmd_line"));
				configFile = decode(serviceElem.getAttribute("config_file"));
				std::auto_ptr<char> portStr(decode(serviceElem.getAttribute("port")));
				port = atol(portStr.get());
				args = decode(serviceElem.getAttribute("args"));
			}
		}
		catch (...)
		{
			throw AdminException("Some exception occured");
		}
	}

	~CommandAddService()
		throw()
	{
		if (cmdLine != 0)
		{
			delete[] cmdLine;
		}
		if (configFile != 0)
		{
			delete[] configFile;
		}
		if (args != 0)
		{
			delete[] args;
		}
	}

	const char * const getCmdLine() const throw()
	{
		return cmdLine;
	}

	const char * const getConfigFileName() const throw()
	{
		return configFile;
	}

	const char * const getArgs() const throw ()
	{
		return args;
	}

	const in_port_t getPort() const throw ()
	{
		return port;
	}
protected:
	char* cmdLine;
	char* configFile;
	in_port_t port;
	char* args;
};

}
}
}

#endif // ifndef SMSC_ADMIN_PROTOCOL_COMMAND_ADD_SERVICE

