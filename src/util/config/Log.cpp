#include "Log.h"
#include "XmlUtils.h"

namespace smsc   {
namespace util   {
namespace config {

const DOMString Log::location_name(createDOMString("location"));

Log::Log(DOM_Element & config_node) : node(config_node)
{
	location = config_node.getAttribute(location_name).transcode();
}

void Log::setLocation(const char * const new_location)
{
	if (new_location != 0)
	{
		node.setAttribute(location_name, new_location);
	}
}

}
}
}

