#include "Command.h"

#include <util/Logger.h>

namespace smsc {
namespace admin {
namespace protocol {

const Command::_Command Command::names[commands_quantity] = {
	{"undefined",      Command::undefined},
	{"get_config",     Command::get_config},
	{"set_config",     Command::set_config},
	{"get_logs",       Command::get_logs},
	{"get_monitoring", Command::get_monitoring},
	{"shutdown",       Command::shutdown}
};

Command::Command()
	: logger(smsc::util::Logger::getCategory("smsc.admin.protocol.command"))
{
	id = undefined;
}

Command::~Command()
{
}

Command::Id Command::getCommandIdByName(const char * const name)
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

