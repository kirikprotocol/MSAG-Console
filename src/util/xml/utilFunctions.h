#ifndef SMSC_UTIL_XML_UTIL_FUNCTIONS
#define SMSC_UTIL_XML_UTIL_FUNCTIONS

#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/TransService.hpp>
#include <memory>

namespace smsc {
namespace util {
namespace xml {

using namespace xercesc;
char * getNodeText(const DOMNode &node);
char * getNodeAttribute(const DOMNode &node, const char * const attrName);
const char * const getLocalEncoding();


class XmlStr
{
public:
  XmlStr(const XMLCh* const str);
  XmlStr(const char * const str);
  ~XmlStr();

  const char * const c_str();
  const XMLCh * const x_str();
  char * c_release();
  XMLCh * x_release();

  bool operator==(const char* str)
  {
    return !strcmp(c_str(),str);
  }

  operator const char  *() {return c_str();}
  operator const XMLCh *() {return x_str();}

private:
  const char * cstr;
  const XMLCh * xstr;
  bool xown;
  bool released;
};

}
}
}
#endif // ifndef SMSC_UTIL_XML_UTIL_FUNCTIONS
