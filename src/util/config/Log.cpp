#include "Log.h"
#include "XmlUtils.h"

namespace smsc   {
namespace util   {
namespace config {

Log::Log(DOM_Node & config_node) {
	location = getNodeAttributeByName(config_node, "location");
	node = &config_node;
}

void Log::setLocation(const char * const new_location)
{
}

}
}
}

