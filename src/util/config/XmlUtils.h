#ifndef SMSC_UTIL_CONFIG_XMLUTILS
#define SMSC_UTIL_CONFIG_XMLUTILS

#include <xercesc/dom/DOM_Node.hpp>
#include <xercesc/dom/DOM_Element.hpp>
#include <string.h>

namespace smsc   {
namespace util   {
namespace config {

DOMString & createDOMString(const char * const str);

#define replaceString(destination, source) {\
	delete[] destination;\
	destination = new char[strlen(source)+1];\
	strcpy(destination, source);\
}

}
}
}

#endif // !SMSC_UTIL_CONFIG_XMLUTILS
