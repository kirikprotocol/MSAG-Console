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
	CommandAddService(const DOMDocument *doc)	throw (AdminException)
		: CommandService(add_service, doc)
	{
		smsc_log_debug(logger, "Add service command");
		try
		{
			DOMElement *elem = doc->getDocumentElement();
			DOMNodeList *list = elem->getElementsByTagName(XmlStr("service"));
			if (list->getLength() > 0)
			{
				DOMElement *serviceElem = (DOMElement*) list->item(0);
				serviceId.reset(XmlStr(serviceElem->getAttribute(XmlStr("id"))).c_release());
				args.reset(XmlStr(serviceElem->getAttribute(XmlStr("args"))).c_release());
				autostart = strcmp("true", XmlStr(serviceElem->getAttribute(XmlStr("autostart")))) == 0;
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

	const bool isAutostart() const throw ()
	{
		return autostart;
	}


protected:
	std::auto_ptr<char> args;
	bool autostart;
};

}
}
}

#endif // ifndef SMSC_ADMIN_PROTOCOL_COMMAND_ADD_SERVICE

