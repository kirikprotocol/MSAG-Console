#ifndef SMSC_UTIL_CONFIG_XMLUTILS
#define SMSC_UTIL_CONFIG_XMLUTILS

#include <xercesc/dom/DOM_Node.hpp>
#include <xercesc/dom/DOM_Element.hpp>

namespace smsc   {
namespace util   {
namespace config {

DOMString & createDOMString(const char * const str);

}
}
}

#endif // !SMSC_UTIL_CONFIG_XMLUTILS
