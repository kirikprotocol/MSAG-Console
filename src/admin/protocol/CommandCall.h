#ifndef	SMSC_ADMIN_PROTOCOL_COMMAND_RUN_COMMAND
#define SMSC_ADMIN_PROTOCOL_COMMAND_RUN_COMMAND

#include <xercesc/dom/DOM.hpp>
#include <admin/protocol/Command.h>
#include <admin/protocol/CommandService.h>
#include <admin/service/Method.h>
#include <admin/service/Type.h>
#include <admin/service/Variant.h>
#include <logger/Logger.h>
#include <util/xml/utilFunctions.h>

namespace smsc {
namespace admin {
namespace protocol {

using namespace xercesc;
using smsc::admin::protocol::CommandService;
//using smsc::admin::service::ServiceCommand;
using smsc::util::xml::getNodeAttribute;
using smsc::logger::Logger;
using smsc::admin::service::Method;
using smsc::admin::service::Methods;
using smsc::admin::service::Variant;
using smsc::admin::service::Arguments;
using smsc::admin::service::cstr2Type;

class CommandCall : public CommandService
{
public:
	CommandCall(const DOMDocument *doc)
		throw ()
		: CommandService(call, doc),
			method(doc->getDocumentElement()),
		  logger(Logger::getInstance("smsc.admin.service.ServiceCommand"))
	{
		DOMElement *elem = data->getDocumentElement();
		component	= XmlStr(elem->getAttribute(XmlStr("component"))).c_release();
		DOMNodeList *params = elem->getElementsByTagName(XmlStr("param"));
    unsigned paramsLength = params->getLength();
		for (unsigned i=0; i<paramsLength; i++)
		{
			DOMNode *paramNode = params->item(i);
			DOMElement * paramElem = (DOMElement *) paramNode;
			XmlStr name(paramElem->getAttribute(XmlStr("name")));
			XmlStr type(paramElem->getAttribute(XmlStr("type")));
			std::auto_ptr<char> value(getNodeText(*paramElem));
			switch (cstr2Type(type.c_str()))
			{
			case service::StringType:
				args[name.c_str()] = Variant(value.get());
				break;
			case service::LongType:
				args[name.c_str()] = Variant(atol(value.get()));
			  break;
			case service::BooleanType:
				args[name.c_str()] = Variant( (strcmp(value.get(),  "true") == 0)
																	 || (strcmp(value.get(),  "on") == 0)
																	 || (strcmp(value.get(),  "1") == 0)
																	 || (strcmp(value.get(),  "-1") == 0)
																	 || (strcmp(value.get(),  "yes") == 0));
			  break;
      case service::StringListType:
        args[name.c_str()] = Variant(value.get(), service::StringListType);
			  break;
			default:
				smsc_log_debug(logger, "creating ServiceCommand: unknown parameter type %s", type.c_str());
			}
		}
	}

	~CommandCall()
		throw ()
	{
		if (component != 0)
		{
			delete[] component;
		}
	}

	const char * const getComponentName() const
	{
		return component;
	}

	const Method &getMethod() const
	{
		return method;
	}

	const Arguments &getArgs() const
	{
		return args;
	}

protected:
	char * component;
	Method method;
	Arguments args;
	smsc::logger::Logger *logger;
//	ServiceCommand *serviceCommand;
};

}
}
}
#endif
