#ifndef SMSC_UTIL_XML_DTDRESOLVER
#define SMSC_UTIL_XML_DTDRESOLVER

#include <sys/stat.h>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>
#include "core/buffers/TmpBuf.hpp"
#include "util/xml/utilFunctions.h"
#include "util/AutoArrPtr.hpp"

#if XERCES_VERSION_MAJOR > 2
#include <xercesc/util/XMLEntityResolver.hpp>
#include <xercesc/sax/InputSource.hpp>
#endif

namespace smsc {
namespace util {
namespace xml {

XERCES_CPP_NAMESPACE_USE
using smsc::logger::Logger;
using smsc::core::buffers::TmpBuf;
using namespace smsc::util::xml;

#if XERCES_VERSION_MAJOR > 2
class DtdResolver : public XMLEntityResolver
#else
class DtdResolver : public DOMEntityResolver
#endif
{
public:
  DtdResolver()
    :logger(smsc::logger::Logger::getInstance("smsc.util.xml.DtdResolver"))
  {
  }
  virtual ~DtdResolver() {}

#if XERCES_VERSION_MAJOR > 2
  virtual InputSource* resolveEntity(XMLResourceIdentifier* resourceIdentifier) {

    const XMLCh* const systemId = resourceIdentifier->getSystemId();

#else
  virtual DOMInputSource* resolveEntity(const XMLCh* const publicId, const XMLCh* const systemId, const XMLCh* const baseURI) {
#endif

    if (XMLString::endsWith(systemId, XmlStr(".dtd")))
    {
      int idx = XMLString::lastIndexOf(systemId, chForwardSlash);
      if (idx < 0)
        idx = XMLString::lastIndexOf(systemId, chBackSlash);
      if (idx >= 0)
      {
        size_t len = XMLString::stringLen(systemId);
        TmpBuf<XMLCh,128> tmp(len+1);
        XMLString::subString(tmp, systemId, idx+1, len);
        return createInputSource(tmp);
      } else {
        return createInputSource(systemId);
      }
    } else
      return 0;
  }

private:
  Logger *logger;

#if XERCES_VERSION_MAJOR > 2
  virtual InputSource*
#else
  virtual DOMInputSource*
#endif
    tryPrefix(const XMLCh * const dtdName, const char * const prefixChars)
  {
    struct stat s;
    const size_t prefixLen = strlen(prefixChars);
    const size_t dtdNameLen = XMLString::stringLen(dtdName);
    TmpBuf<XMLCh,128> tmpDtdName(prefixLen + dtdNameLen + 1);
    XMLString::copyNString(tmpDtdName, XmlStr(prefixChars), (const unsigned)prefixLen);
    XMLString::copyNString(tmpDtdName + prefixLen, dtdName, (const unsigned)dtdNameLen);
    tmpDtdName[prefixLen + dtdNameLen] = 0;

    smsc::util::auto_arr_ptr<char> dtdNameTranscodedToCallCFunctionStat(XMLString::transcode(tmpDtdName));
    if (stat(dtdNameTranscodedToCallCFunctionStat.get(), &s) == 0)
    {
#if XERCES_VERSION_MAJOR > 2
      return new LocalFileInputSource(tmpDtdName);
#else
      return new Wrapper4InputSource(new LocalFileInputSource(tmpDtdName));
#endif
    }
    else
      return 0;
  }

#if XERCES_VERSION_MAJOR > 2
  virtual InputSource*
#else
  virtual DOMInputSource*
#endif
    createInputSource(const XMLCh * const dtdName)
  {
    // const char prefixChars[] = "../conf/";

#if XERCES_VERSION_MAJOR > 2
    InputSource* result = 0;
#else
    DOMInputSource* result = 0;
#endif
    if ((result = tryPrefix(dtdName, "../conf/")))   return result;
    else if ((result = tryPrefix(dtdName, "conf/"))) return result;
    else
#if XERCES_VERSION_MAJOR > 2
      return new LocalFileInputSource(dtdName);
#else
      return new Wrapper4InputSource(new LocalFileInputSource(dtdName));
#endif
  }
};


}
}
}
#endif // ifndef SMSC_UTIL_XML_DTDRESOLVER
