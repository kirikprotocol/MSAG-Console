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
		smsc_log_debug(logger, "Add service command");
		try
		{
			DOMElement *elem = doc->getDocumentElement();
			DOMNodeList *list = elem->getElementsByTagName(XmlStr("service"));
			if (list->getLength() > 0)
			{
				DOMElement *serviceElem = (DOMElement*) list->item(0);
				//serviceName.reset(serviceElem.getAttribute("name").transcode());
				serviceId.reset(XmlStr(serviceElem->getAttribute(XmlStr("id"))).c_release());
				port = atol(XmlStr(serviceElem->getAttribute(XmlStr("port"))));
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

	const in_port_t getPort() const throw ()
	{
		return port;
	}

	const bool isAutostart() const throw ()
	{
		return autostart;
	}


protected:
	/*std::auto_ptr<char> serviceName;*/
	in_port_t port;
	std::auto_ptr<char> args;
	bool autostart;
};

}
}
}

#endif // ifndef SMSC_ADMIN_PROTOCOL_COMMAND_SET_SERVICE_STARTUP_PARAMETERS

