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
				DOM_Node serviceNode = list.item(0);
				DOM_Element &serviceElem = (DOM_Element&) serviceNode;
				//serviceName.reset(serviceElem.getAttribute("name").transcode());
				serviceId.reset(serviceElem.getAttribute("id").transcode());
				std::auto_ptr<char> portStr(serviceElem.getAttribute("port").transcode());
				port = atol(portStr.get());
				args.reset(serviceElem.getAttribute("args").transcode());
			}
		}
		catch (...)
		{
			throw AdminException("Some exception occured");
		}
	}

	/*const char * const getServiceName() const throw()
	{
		return serviceName.get();
	}*/

	const char * const getArgs() const throw ()
	{
		return args.get();
	}

	const in_port_t getPort() const throw ()
	{
		return port;
	}


protected:
	//std::auto_ptr<char> serviceName;
	in_port_t port;
	std::auto_ptr<char> args;
};

}
}
}

#endif // ifndef SMSC_ADMIN_PROTOCOL_COMMAND_ADD_SERVICE

