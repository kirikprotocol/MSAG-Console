#ifndef	SMSC_ADMIN_PROTOCOL_COMMAND_RUN_COMMAND
#define SMSC_ADMIN_PROTOCOL_COMMAND_RUN_COMMAND

#include <log4cpp/Category.hh>
#include <xercesc/dom/DOM_Document.hpp>
#include <xercesc/dom/DOM_Node.hpp>
#include <xercesc/dom/DOM_NodeList.hpp>
#include <xercesc/dom/DOM_Element.hpp>
#include <admin/protocol/Command.h>
#include <admin/protocol/CommandService.h>
#include <admin/service/Method.h>
#include <admin/service/Type.h>
#include <admin/service/Variant.h>
#include <util/Logger.h>
#include <util/xml/utilFunctions.h>
//#include <admin/service/ServiceCommand.h>

namespace smsc {
namespace admin {
namespace protocol {

using smsc::admin::protocol::CommandService;
//using smsc::admin::service::ServiceCommand;
using smsc::util::xml::getNodeAttribute;
using smsc::util::Logger;
using smsc::admin::service::Method;
using smsc::admin::service::Methods;
using smsc::admin::service::Variant;
using smsc::admin::service::Arguments;
using smsc::admin::service::cstr2Type;

class CommandCall : public CommandService
{
public:
	CommandCall(DOM_Document doc)
		throw ()
		: CommandService(call, doc),
			method(doc.getDocumentElement()),
		  logger(Logger::getCategory("smsc.admin.service.ServiceCommand"))
	{
		DOM_Element elem = data.getDocumentElement();
		component	= elem.getAttribute("component").transcode();

		DOM_NodeList params = elem.getElementsByTagName("param");
		for (unsigned i=0; i<params.getLength(); i++)
		{
			DOM_Node paramNode = params.item(i);
			DOM_Element & paramElem = (DOM_Element &) paramNode;
			std::auto_ptr<char> name(paramElem.getAttribute("name").transcode());
			std::auto_ptr<char> type(paramElem.getAttribute("type").transcode());
			std::auto_ptr<char> value(getNodeText(paramElem));
			switch (cstr2Type(type.get()))
			{
			case service::StringType:
				args[name.get()] = Variant(value.get());
				break;
			case service::LongType:
				args[name.get()] = Variant(atol(value.get()));
			case service::BooleanType:
				args[name.get()] = Variant(   (strcmp(value.get(),  "true") == 0)
																	 || (strcmp(value.get(),  "on") == 0)
																	 || (strcmp(value.get(),  "1") == 0)
																	 || (strcmp(value.get(),  "-1") == 0)
																	 || (strcmp(value.get(),  "yes") == 0));
			default:
				logger.debug("creating ServiceCommand: unknown parameter type %s",
										 type.get());
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
	log4cpp::Category &logger;
//	ServiceCommand *serviceCommand;
};

}
}
}
#endif
