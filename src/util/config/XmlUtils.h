#ifndef SMSC_UTIL_CONFIG_XMLUTILS
#define SMSC_UTIL_CONFIG_XMLUTILS

#include <xercesc/dom/DOM_Node.hpp>
#include <xercesc/dom/DOM_Element.hpp>

DOM_Node getElementChildByTagName(const DOM_Element & node, const char * const name);
char * getNodeAttributeByName(const DOM_Node & node, const char * const name);

#endif // !SMSC_UTIL_CONFIG_XMLUTILS
