#include "MapProtocol.h"
#include "XmlUtils.h"

namespace smsc   {
namespace util   {
namespace config {

const DOMString MapProtocol::hlr_name(createDOMString("hlr_address"));
const DOMString MapProtocol::vlr_name(createDOMString("vlr_address"));

MapProtocol::MapProtocol(DOM_Element & config_node) : node(config_node)
{	
	hlr = config_node.getAttribute(hlr_name).transcode();
	vlr = config_node.getAttribute(vlr_name).transcode();
}

void MapProtocol::setHLRAddress(const char * const address)
{
	node.setAttribute(hlr_name, address);
}

void MapProtocol::setVLRAddress(const char * const address)
{
	node.setAttribute(vlr_name, address);
}

}
}
}

