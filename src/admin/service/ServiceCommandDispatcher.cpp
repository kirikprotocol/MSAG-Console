#include "ServiceCommandDispatcher.h"

#include <log4cpp/NDC.hh>

#include <admin/protocol/Command.h>
#include <admin/protocol/CommandCall.h>
#include <admin/protocol/CommandListComponents.h>
#include <admin/protocol/Response.h>
#include <admin/service/ComponentManager.h>
#include <core/synchronization/Mutex.hpp>
#include <util/config/Config.h>
#include <util/Logger.h>
#include <util/cstrings.h>

namespace smsc {
namespace admin {
namespace service {

using smsc::admin::protocol::CommandCall;
using smsc::admin::protocol::CommandListComponents;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;
using smsc::util::encode;

Response * ServiceCommandDispatcher::handle(const Command * const command)
	throw (AdminException)
{
	if (command == 0)
	{
		throw AdminException("Command is null");
	}

	try
	{
		Response * response = 0;
	
		switch (command->getId())
		{
		case Command::call:
			response = call((const CommandCall * const) command);
			break;
		case Command::list_components:
			response = listComponents((const CommandListComponents * const) command);
			break;
		default:
			logger.warn("error in parsing: unknown command");
		}
	
		return response;
	}
	catch (AdminException &e)
	{
		throw e;
	}
	catch (...) {
		throw AdminException("Unknown exception in command dispatcher");
	}
}

Response * ServiceCommandDispatcher::call(const CommandCall * const command)
	throw (AdminException)
{
	return new Response(Response::Ok,
											ComponentManager::dispatch(command->getComponentName(),
																								 command->getMethod(),
																								 command->getArgs())
											);
}

Response * ServiceCommandDispatcher::listComponents(const CommandListComponents * const command)
	throw (AdminException)
{				
	std::string componentsList;
	char * compName;
	Component * comp;
	for (Components::Iterator i = ComponentManager::getComponents().getIterator(); i.Next(compName, comp); )
	{
		std::auto_ptr<char> tmpComponentName(encode(comp->getName()));
		componentsList += "  <component name=\"";
		componentsList += tmpComponentName.get();
		componentsList += "\">\n";
		char * mName;
		Method m;
		for (Methods::Iterator j = comp->getMethods().getIterator(); j.Next(mName, m); )
		{
			std::auto_ptr<char> tmpMethodName(encode(m.getName()));
			componentsList += "    <method name=\"";
			componentsList += tmpMethodName.get();
			componentsList += "\" type=\"";
			const char * const tstr = type2cstr(m.getReturnType());
			if (tstr == 0)
			{
				std::string msg;
				msg += "Unknown method \"";
				msg += m.getName();
				msg += "\" return type in component \"";
				msg += comp->getName();
				msg += "\"";
				throw AdminException(msg.c_str());
			}
			componentsList += tstr;
			componentsList += "\">\n";
			Parameter p;
			char * pName;
			for (Parameters::Iterator k = m.getParameters().getIterator(); k.Next(pName, p);)
			{
				std::auto_ptr<char> tmpParamName(encode(p.getName()));
				componentsList += "      <param name=\"";
				componentsList += tmpParamName.get();
				const char * const typestr = type2cstr(p.getType());
				if (typestr == 0)
				{
					std::string msg;
					msg += "Unknown parameter \"";
					msg += p.getName();
					msg += "\" type in method \"";
					msg += m.getName();
					msg += "\" in component \"";
					msg += comp->getName();
					msg += "\"";
					throw AdminException(msg.c_str());
				}
				componentsList += "\" type=\"";
				componentsList += typestr;
				componentsList += "\"/>\n";
			}
			componentsList += "    </method>\n";
		}
		componentsList += "  </component>\n";
	}
	return new Response(Response::Ok, componentsList.c_str());
}

}
}
}

