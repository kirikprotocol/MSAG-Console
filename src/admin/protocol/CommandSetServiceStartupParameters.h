#ifndef SMSC_ADMIN_PROTOCOL_COMMAND_SET_SERVICE_STARTUP_PARAMETERS
#define SMSC_ADMIN_PROTOCOL_COMMAND_SET_SERVICE_STARTUP_PARAMETERS

namespace smsc {
namespace admin {
namespace protocol {

class CommandSetServiceStartupParameters : public CommandService
{
public:
	CommandSetServiceStartupParameters(const DOMDocument *doc)	throw (AdminException)
		: CommandService(set_service_startup_parameters, doc)
	{
		smsc_log_debug(logger, "Set service startup parameters command");
		try
		{
			DOMElement *elem = doc->getDocumentElement();
			DOMNodeList *list = elem->getElementsByTagName(XmlStr("service"));
			if (list->getLength() > 0)
			{
				DOMElement *serviceElem = (DOMElement*) list->item(0);
				//serviceName.reset(serviceElem.getAttribute("name").transcode());
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

#endif // ifndef SMSC_ADMIN_PROTOCOL_COMMAND_SET_SERVICE_STARTUP_PARAMETERS

