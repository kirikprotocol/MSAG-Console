#ifndef MAPCONFIGURATION_H_INCLUDED_C3A831D5
#define MAPCONFIGURATION_H_INCLUDED_C3A831D5

#include <xercesc/dom/DOM_Element.hpp>
#include <xercesc/dom/DOM_DOMException.hpp>

namespace smsc   {
namespace util   {
namespace config {

/**
 * Настройки MAP протокола.
 *
 * @author igork
 * @see Manager
 */
class MapProtocol
{
public:
	MapProtocol(DOM_Element & config_node);
	const char * const getHLRAddress() const {return hlr;};
	const char * const getVLRAddress() const {return vlr;};
	void setHLRAddress(const char * const address) throw (DOM_DOMException);
	void setVLRAddress(const char * const address) throw (DOM_DOMException);

private:
	char *hlr;
	char* vlr;
	DOM_Element node;

	static const DOMString hlr_name;
	static const DOMString vlr_name;
};

}
}
}

#endif /* MAPCONFIGURATION_H_INCLUDED_C3A831D5 */
