#ifndef SMSC_ADMIN_PROTOCOL_COMMAND_SET_SERVICE_STARTUP_PARAMETERS
#define SMSC_ADMIN_PROTOCOL_COMMAND_SET_SERVICE_STARTUP_PARAMETERS

namespace smsc {
namespace admin {
namespace protocol {

class CommandSetServiceStartupParameters : public CommandService
{
public:
	CommandSetServiceStartupParameters(DOM_Document doc)	throw (AdminException)
		: CommandService(set_service_startup_parameters, doc)
	{
		smsc_log_debug(logger, "Add service command");
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
				std::auto_ptr<char> autostartStr(serviceElem.getAttribute("autostart").transcode());
				autostart = strcmp("true", autostartStr.get()) == 0;
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

