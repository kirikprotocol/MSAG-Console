#include "Log.h"
#include "XmlUtils.h"

namespace smsc   {
namespace util   {
namespace config {

const DOMString Log::location_name(createDOMString("location"));

Log::Log(DOM_Element & config_node)
{
	node = config_node;
	location = node.getAttribute(location_name).transcode();
}

void Log::setLocation(const char * const newLocation)
	throw (DOM_DOMException)
{
	node.setAttribute(location_name, newLocation);
	replaceString(location, newLocation);
}

}
}
}

