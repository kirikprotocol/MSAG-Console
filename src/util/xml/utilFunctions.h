#ifndef SMSC_UTIL_XML_UTIL_FUNCTIONS
#define SMSC_UTIL_XML_UTIL_FUNCTIONS

#include <xercesc/dom/DOM_Node.hpp>

namespace smsc {
namespace util {
namespace xml {

char * getNodeText(DOM_Node node);
char * getNodeAttribute(DOM_Node node, const char * const attrName);

}
}
}
#endif // ifndef SMSC_UTIL_XML_UTIL_FUNCTIONS

