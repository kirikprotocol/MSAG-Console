#ifndef SMSC_ADMIN_SERVICE_PROTOCOL_SERVICE_COMMAND
#define SMSC_ADMIN_SERVICE_PROTOCOL_SERVICE_COMMAND

/*#include <xercesc/dom/DOM_Element.hpp>
#include <xercesc/dom/DOM_Node.hpp>
#include <xercesc/dom/DOM_NodeList.hpp>
#include <admin/service/Method.h>
#include <admin/service/Variant.h>
#include <util/xml/utilFunctions.h>
#include <util/Logger.h>

namespace smsc {
namespace admin {
namespace service {

using smsc::util::xml;
using smsc::util::Logger;

class ServiceCommand
{
protected:
	Method method;
	Arguments args;
	log4cpp::Category &logger;

public:
	ServiceCommand(DOM_Node commandNode)
		throw (AdminException &)
		: method(commandNode),
		  logger(Logger::getCategory("smsc.admin.service.ServiceCommand"))
	{
		DOM_NodeList childs = commandNode.getChildNodes();
		for (unsigned i=0; i<childs.getLength(); i++)
		{
			DOM_Node node = childs.item(i);
			if (node.getNodeType() == DOM_Node::ELEMENT_NODE)
			{
				DOM_Element &childElem = (DOM_Element&)node;
				std::auto_ptr<char> name(childElem.getAttribute("name").transcode());
				std::auto_ptr<char> type(childElem.getAttribute("type").transcode());
				std::auto_ptr<char> value(getNodeText(childElem));
				switch (cstr2Type(type.get()))
				{
				case StringType:
					args[name.get()] = Variant(value.get());
					break;
				case LongType:
					args[name.get()] = Variant(atol(value.get()));
				case BooleanType:
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
	}

	const Method &getMethod() const
	{
		return method;
	}

	const Arguments &getArgs() const
	{
		return args;
	}
};

}
}
}*/
#endif
