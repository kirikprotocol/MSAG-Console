#ifndef SMSC_ADMIN_PROTOCOL_COMMAND_ADD_SERVICE
#define SMSC_ADMIN_PROTOCOL_COMMAND_ADD_SERVICE

#include <admin/AdminException.h>
#include <admin/protocol/CommandService.h>
#include <util/xml/utilFunctions.h>
#include <admin/daemon/Service.h>

namespace smsc {
namespace admin {
namespace protocol {

using smsc::admin::AdminException;
using smsc::admin::daemon::ServiceArguments;
using smsc::util::xml::getNodeAttribute;
using smsc::util::xml::getNodeText;

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
				serviceName = serviceElem.getAttribute("name").transcode();
				cmdLine = serviceElem.getAttribute("cmd_line").transcode();
				configFile = serviceElem.getAttribute("config_file").transcode();
				std::auto_ptr<char> portStr(serviceElem.getAttribute("port").transcode());
				port = atol(portStr.get());
	
				DOM_NodeList argElems = serviceElem.getElementsByTagName("arg");
				args.resize(argElems.getLength());
				for (int i=0; i<argElems.getLength(); i++)
				{
					DOM_Node argNode = argElems.item(i);
					DOM_Element &argElem = (DOM_Element&) argNode;
					std::auto_ptr<char> argNumStr(argElem.getAttribute("num").transcode());
					try
					{
						args.at(atoi(argNumStr.get())) = getNodeText(argElem);
					}
					catch (std::out_of_range & o)
					{
						throw AdminException("Wrong argument number");
					}
				}
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
		for (size_t i=0; i<args.size(); i++)
		{
			if (args[i] != 0)
			{
				delete[] args[i];
				args[i]=0;
			}
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

	const ServiceArguments getArgs() const throw ()
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
	ServiceArguments args;
};

}
}
}

#endif // ifndef SMSC_ADMIN_PROTOCOL_COMMAND_ADD_SERVICE

