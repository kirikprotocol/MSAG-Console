#include "MapProtocol.h"
#include "XmlUtils.h"

namespace smsc   {
namespace util   {
namespace config {

const DOMString MapProtocol::hlr_name(createDOMString("hlr_address"));
const DOMString MapProtocol::vlr_name(createDOMString("vlr_address"));

MapProtocol::MapProtocol(DOM_Element & config_node)
{	
	node = config_node;
	hlr = node.getAttribute(hlr_name).transcode();
	vlr = node.getAttribute(vlr_name).transcode();
}

void MapProtocol::setHLRAddress(const char * const address)
	throw (DOM_DOMException)
{
	node.setAttribute(hlr_name, address);
	replaceString(hlr, address)
}

void MapProtocol::setVLRAddress(const char * const address)
	throw (DOM_DOMException)
{
	node.setAttribute(vlr_name, address);
	replaceString(vlr, address);
}

}
}
}

