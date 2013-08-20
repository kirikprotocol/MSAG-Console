#include "utilFunctions.h"

#include <string>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <util/cstrings.h>
#include <util/Exception.hpp>
#include "core/buffers/TmpBuf.hpp"
#include "util/AutoArrPtr.hpp"

namespace smsc {
namespace util {
namespace xml {

using namespace xercesc;
using namespace smsc::util;

//static const char * const INTERNAL_SMSC_ENCODING = "WINDOWS-1251";

char * getNodeText(const DOMNode &node)
{
  std::string s;

  DOMNodeList *childs = node.getChildNodes();
  size_t childsLength = childs->getLength();
  for (size_t i=0; i<childsLength; i++)
  {
    DOMNode * child = childs->item(i);
    if (child->getNodeType() == DOMNode::TEXT_NODE)
    {
      s += XmlStr(child->getNodeValue());
    }
  }
  return cStringCopy(s.c_str());
}

char * getNodeAttribute(const DOMNode &node, const char * const attrName)
{
  DOMNamedNodeMap *attrs = node.getAttributes();
  DOMNode *a = attrs->getNamedItem(XmlStr(attrName));
  if (a != NULL)
    return XmlStr(a->getNodeValue()).c_release();
  else
    return 0;
}

const char * encName = 0;
const char * const getLocalEncoding()
{
  if (encName == 0)
  {
    const char* enc =getenv("EYELINE_ICONV_ENCODING");
    if(!enc)
    {
      enc=getenv("LC_ALL");
    }
    if(!enc)
    {
      enc = getenv("LC_CTYPE");
    }
    if (!enc)
    {
      enc = getenv("LANG");
    }
    if (enc && strrchr(enc, '.'))
    {
      encName = strrchr(enc, '.')+1;
    }else
    {
      encName = enc;
    }
    encName = encName ? cStringCopy(encName) : "WINDOWS-1251";
  }
  return encName;
}

XmlStr::XmlStr(const XMLCh* const str)
  : cstr(0), xstr(str), xown(false), released(false)
{
  const char* const INTERNAL_SMSC_ENCODING = getLocalEncoding();
  XMLTransService::Codes resValue = XMLTransService::Ok;
  std::auto_ptr<XMLTranscoder> transcoder(XMLPlatformUtils::fgTransService->makeNewTranscoderFor(INTERNAL_SMSC_ENCODING, resValue, XMLSize_t(0x7FFF)));
  if (resValue != XMLTransService::Ok)
    throw Exception("could not create transcoder for internal SMSC encoding (\"%s\")", INTERNAL_SMSC_ENCODING);

#if XERCES_VERSION_MAJOR > 2
  const XMLSize_t srcCount = XMLString::stringLen(str);
  XMLSize_t charsEaten = 0;
  XMLSize_t res = 0;
  const XMLSize_t ERRVAL = (XMLSize_t)-1;
#else
  const unsigned int srcCount = XMLString::stringLen(str);
  unsigned int charsEaten = 0;
  unsigned int res = 0;
  const unsigned int ERRVAL = (unsigned int)-1;
#endif

  smsc::core::buffers::TmpBuf<char,128> tmpbuf;
  tmpbuf.setSize(srcCount*4+16);
  // size_t cstrLen = srcCount*5 + 16;
  // cstr = new char[cstrLen+1];
#if XERCES_VERSION_MAJOR > 2
  const XMLSize_t dstCount = tmpbuf.getSize();
#else
  const unsigned int dstCount = (unsigned int)tmpbuf.getSize();
#endif

  res = transcoder->transcodeTo(str, srcCount, (unsigned char * const) tmpbuf.get(), dstCount, charsEaten, XMLTranscoder::UnRep_RepChar);

  if (res == ERRVAL)
    throw Exception("Could not transcode string");
  else if (charsEaten != srcCount) {
      throw Exception("Could not transcode string, input=%u, eaten=%u",srcCount,charsEaten);
  }
  char* buf = new char[res+1];
  cstr = buf;
  memcpy(buf,tmpbuf.get(),res);
  buf[res] = '\0';
}

XmlStr::XmlStr(const char* const str)
  :cstr(str), xstr(0), xown(true), released(false)
{
  const char* const INTERNAL_SMSC_ENCODING = getLocalEncoding();
  XMLTransService::Codes resValue = XMLTransService::Ok;
  std::auto_ptr<XMLTranscoder> transcoder(XMLPlatformUtils::fgTransService->makeNewTranscoderFor(INTERNAL_SMSC_ENCODING, resValue, XMLSize_t(0x7FFF)));
  if (resValue != XMLTransService::Ok)
    throw Exception("could not create transcoder for internal SMSC encoding (\"%s\")", INTERNAL_SMSC_ENCODING);

#if XERCES_VERSION_MAJOR > 2
  const XMLSize_t srcCount = XMLString::stringLen(str);
  XMLSize_t charsEaten = 0;
  XMLSize_t res = 0;
  const XMLSize_t ERRVAL = (XMLSize_t)-1;
#else
  const unsigned int srcCount = XMLString::stringLen(str);
  unsigned int charsEaten = 0;
  unsigned int res = 0;
  const unsigned int ERRVAL = (unsigned int)-1;
#endif

  size_t xstrLen = srcCount + 16;
  xstr = new XMLCh[xstrLen+1];

#if XERCES_VERSION_MAJOR > 2
  const XMLSize_t dstCount = (XMLSize_t)xstrLen;
#else
  const unsigned int dstCount = (unsigned int)xstrLen;
#endif

  smsc::util::auto_arr_ptr<unsigned char> charSizes(new unsigned char[xstrLen +1]);

  res = transcoder->transcodeFrom((const unsigned char* const) str, srcCount, (XMLCh* const)xstr, dstCount, charsEaten, (unsigned char* const)charSizes.get());

  if (res == ERRVAL)
    throw Exception("Could not transcode string");
}

XmlStr::~XmlStr()
{
  if (!released) {
    if (xown)
      delete [] xstr;
    else
      delete [] cstr;
  }
}

const char * const XmlStr::c_str()const
{
  return cstr;
}

const XMLCh * const XmlStr::x_str()
{
  return xstr;
}

char * XmlStr::c_release()
{
  if (xown)
  {
    released = false;
    return cStringCopy(cstr);
  }
  else
  {
    released = true;
    return (char *)cstr;
  }
}

XMLCh * XmlStr::x_release()
{
  if (xown)
  {
    released = true;
    return (XMLCh*)xstr;
  }
  else
  {
    released = false;
    return XMLString::replicate(xstr);
  }
}


}
}
}
