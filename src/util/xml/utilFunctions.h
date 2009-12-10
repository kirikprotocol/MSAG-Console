#ifndef SMSC_UTIL_XML_UTIL_FUNCTIONS
#define SMSC_UTIL_XML_UTIL_FUNCTIONS

#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/TransService.hpp>
#include <memory>
#include "core/buffers/FixedLengthString.hpp"
#include "util/Exception.hpp"

namespace smsc {
namespace util {
namespace xml {

namespace buf=smsc::core::buffers;

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

  const char * const c_str()const;
  const XMLCh * const x_str();
  char * c_release();
  XMLCh * x_release();

  bool operator==(const char* str)const
  {
    return !strcmp(c_str(),str);
  }

  bool operator!=(const char* str)const
  {
    return strcmp(c_str(),str);
  }

  operator const char  *() {return c_str();}
  operator const XMLCh *() {return x_str();}

private:
  const char * cstr;
  const XMLCh * xstr;
  bool xown;
  bool released;
};

template <size_t SZ>
inline
void AttrFillStringValue(DOMNamedNodeMap* attr,const char* attrName,buf::FixedLengthString<SZ>& str)
{
  XmlStr value(attr->getNamedItem(XmlStr(attrName))->getNodeValue());
  str=value.c_str();
}

template <int SZ>
inline
void AttrFillStringValue(DOMNamedNodeMap* attr,const char* attrName,char (&str)[SZ])
{
  XmlStr value(attr->getNamedItem(XmlStr(attrName))->getNodeValue());
  strncpy(str,value.c_str(),SZ);
  str[SZ-1]=0;
}

inline
int AttrGetIntValue(DOMNamedNodeMap* attr,const char* attrName)
{
  XmlStr value(attr->getNamedItem(XmlStr(attrName))->getNodeValue());
  return atoi(value.c_str());
}

inline
bool AttrGetBoolValue(DOMNamedNodeMap* attr,const char* attrName)
{
  char value[16];
  AttrFillStringValue(attr,attrName,value);
  size_t len=strlen(value);
  for(size_t i=0;i<len;i++)value[i]=tolower(value[i]);
  if     (!strcmp(value,"true")) return true;
  else if(!strcmp(value,"false")) return false;
  throw smsc::util::Exception("Invalid value for boolean attribute '%s':%s",attrName,value);
}


}
}
}
#endif // ifndef SMSC_UTIL_XML_UTIL_FUNCTIONS
