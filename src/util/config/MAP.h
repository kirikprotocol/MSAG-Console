#ifndef MAPCONFIGURATION_H_INCLUDED_C3A831D5
#define MAPCONFIGURATION_H_INCLUDED_C3A831D5

#include <xercesc/dom/DOM_Node.hpp>

namespace smsc   {
namespace util   {
namespace config {

class MAP
{
public:
	const char *getHLRAdress() const;
	const char *getVLRAddress() const;
	void setHLRAddress(const char * const address);
	void setVLRAddress(const char * const address);

private:
	char *hlr;
	char* vlr;
	DOM_Node node;
};

}
}
}

#endif /* MAPCONFIGURATION_H_INCLUDED_C3A831D5 */
