#include "Command.h"

#include <util/Logger.h>

namespace smsc {
namespace admin {
namespace protocol {

const Command::_Command Command::names[commands_quantity] = {
	{"undefined",        Command::undefined},
/*	{"get_config",       Command::get_config},
	{"set_config",       Command::set_config},
	{"get_logs",         Command::get_logs},
	{"get_monitoring",   Command::get_monitoring},*/
	{"start_service",    Command::start_service},
	{"shutdown_service", Command::shutdown_service},
	{"kill_service",     Command::kill_service},
	{"add_service",      Command::add_service},
	{"remove_service",   Command::remove_service},
	{"list_services",    Command::list_services},
	{"list_components",  Command::list_components},
	{"call",             Command::call}
};

Command::Id Command::getCommandIdByName(const char * const name) throw ()
{
	for (int i=0; i<commands_quantity; i++)
	{
		if (strcmp(name, names[i].name) == 0)
		{
			return names[i].id;
		}
	}
	return undefined;
}

}
}
}

