/* 
 * "@(#)$Id$"
 */
#include "util/xml/DOMErrorLogger.h"

namespace smsc   {
namespace util   {
namespace xml {

#if XERCES_VERSION_MAJOR > 2
class StrX
{
public :
  StrX(const XMLCh* const toTranscode) { fLocalForm = XMLString::transcode(toTranscode);  }

  ~StrX() { XMLString::release(&fLocalForm); }

  const char* localForm() const { return fLocalForm; }

private :
  char*   fLocalForm;
};

void DOMErrorLogger::warning(const SAXParseException& exc)
{
  commonError(exc, Logger::LEVEL_WARN);
}

void DOMErrorLogger::error(const SAXParseException& exc)
{
  commonError(exc, Logger::LEVEL_ERROR);
}

void DOMErrorLogger::fatalError(const SAXParseException& exc)
{
  commonError(exc, Logger::LEVEL_FATAL);
}

void DOMErrorLogger::commonError(const SAXParseException& e, Logger::LogLevel logLvl)
{
  const char* errType = "Warning";
  if (logLvl == Logger::LEVEL_ERROR)
    errType = "Error";
  else if (logLvl == Logger::LEVEL_FATAL)
    errType = "Fatal error";

  StrX fname(e.getSystemId());
  StrX msg(e.getMessage());
  XMLFileLoc ln = e.getLineNumber();
  XMLFileLoc cn = e.getColumnNumber();

  SmscParseException  exc("%s at file =) \"%s\" [l:%u, c:%u]\n  Message: %s",
      errType, fname.localForm(), (unsigned long)ln, (unsigned long)cn, msg.localForm());

  _logger->log(logLvl, exc.what());

  if (logLvl >= _throwLvl)
    throw exc;
}

#else
bool DOMErrorLogger::handleError(const DOMError &domError)
  /* throw(SmscParseException)*/
{
  Logger::LogLevel logLvl = Logger::LEVEL_FATAL;

  const char * s1;
  if (domError.getSeverity() == DOMError::DOM_SEVERITY_WARNING) {
    s1 = "Warning"; logLvl = Logger::LEVEL_WARN;
  } else if (domError.getSeverity() == DOMError::DOM_SEVERITY_ERROR) {
    s1 = "Error"; logLvl = Logger::LEVEL_ERROR;
  } else if (domError.getSeverity() == DOMError::DOM_SEVERITY_FATAL_ERROR)
    s1 = "Fatal";
  else
    s1 = "Unknown Fatal";
    
  char *uri = XMLString::transcode(domError.getLocation()->getURI());
  char *message = XMLString::transcode(domError.getMessage());
  
  SmscParseException  exc("%s at file (( \"%s\" [l:%d, c:%d]\n  Message: %s",
                      s1, uri, domError.getLocation()->getLineNumber(),
                      domError.getLocation()->getColumnNumber(), message);

  _logger->log(logLvl, exc.what());
  
  XMLString::release(&uri);
  XMLString::release(&message);

  if (domError.getSeverity() >= int(_throwLvl))
    throw exc;
  return false;
}
#endif

}
}
}

