#include "Manager.h"

namespace smsc   {
namespace util   {
namespace config {

Manager::Manager(const char * const config_filename)
{
	document = 0;
	db = 0;
	map = 0;
	logger = 0;
}

void Manager::save()
{
}

DB &Manager::get_db() const
{
    return *db;
}

Logger &Manager::get_logger() const
{
    return *logger;
}

MAP &Manager::get_map() const
{
    return *map;
}

}
}
}

