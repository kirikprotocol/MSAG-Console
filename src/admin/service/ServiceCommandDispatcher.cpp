#include "ServiceCommandDispatcher.h"

#include <log4cpp/NDC.hh>

#include <admin/protocol/Command.h>
#include <admin/protocol/CommandCall.h>
#include <admin/protocol/Response.h>
#include <admin/service/ComponentManager.h>
#include <core/synchronization/Mutex.hpp>
#include <util/config/Config.h>
#include <util/Logger.h>

namespace smsc {
namespace admin {
namespace service {

using smsc::admin::protocol::CommandCall;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

Response * ServiceCommandDispatcher::handle(const Command * const command)
	throw (AdminException &)
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
			{
				const CommandCall * const toRun = (const CommandCall * const) command;
				
				response = new Response(Response::Ok,
																ComponentManager::dispatch(toRun->getComponentName(),
																													 toRun->getMethod(),
																													 toRun->getArgs())
																);
			}
			break;
		case Command::list_components:
			{
				std::string componentsList;
				char * compName;
				Component * comp;
				for (Components::Iterator i = ComponentManager::getComponents().getIterator(); i.Next(compName, comp); )
				{
					componentsList += "<component name=\"";
					componentsList += comp->getName();
					componentsList += "\">";
					char * mName;
					Method m;
					for (Methods::Iterator j = comp->getMethods().getIterator(); j.Next(mName, m); )
					{
						componentsList += "<method name=\"";
						componentsList += m.getName();
						componentsList += "\" returnType=\"";
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
						componentsList += "\">";
						Parameter p;
						char * pName;
						for (Parameters::Iterator k = m.getParameters().getIterator(); k.Next(pName, p);)
						{
							componentsList += "<param name=\"";
							componentsList += p.getName();
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
							componentsList += "\" type =\"";
							componentsList += typestr;
							componentsList += "\"/>";
						}
						componentsList += "</method>";
					}
					componentsList += "</component>";
				}
				response = new Response(Response::Ok, componentsList.c_str());
			}
			break;
/*		case Command::get_name:
			{
				const Config & config = handler->getConfig();
				std::auto_ptr<char> data(config.getTextRepresentation());
				response = new Response(Response::Ok, data.get());
			}
			break;
		case Command::set_config:
			{
				Config config(command->getData().getDocumentElement());
				handler->setConfig(config);
				response = new Response(Response::Ok, 0);
				break;
			}
		case Command::get_logs:
			{
				const char * const logs = handler->getLogs(0, 5);
				if (logs != 0)
				{
					response = new Response(Response::Ok, logs);
					delete[] logs;
				}
			}
			break;
		case Command::get_monitoring:
			{
				const MonitoringData& mon = handler->getMonitoring();;
				if ((&mon) != 0)
				{
					response = new Response(Response::Ok, mon.getText());
				}
			}
			break;
		case Command::shutdown_service:
			{
				bool result = handler->shutdown();
				response = new Response(result ? Response::Ok : Response::Error, "");
				if (result && (parent != 0))
				{
					parent->shutdown();
				}
			}
			break;*/
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

void ServiceCommandDispatcher::shutdown()
{
	smsc::admin::util::CommandDispatcher::shutdown();
	logger.debug("shutdown");
}

}
}
}


