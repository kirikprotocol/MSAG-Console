#ifndef CONFIGURATIONMANAGER_H_INCLUDED_C3A81BD4
#define CONFIGURATIONMANAGER_H_INCLUDED_C3A81BD4

#include <xercesc/dom/DOM_Document.hpp>

#include "DB.h"
#include "MAP.h"
#include "Logger.h"

namespace smsc   {
namespace util   {
namespace config {

class Manager
{
public:
	Manager(const char * const config_filename);
	void save();
	DB &get_db() const;
	Logger &get_logger() const;
	MAP &get_map() const;

private:
	DOM_Document *document;
	DB *db;
	MAP *map;
	Logger *logger;
};

}
}
}

#endif /* CONFIGURATIONMANAGER_H_INCLUDED_C3A81BD4 */
