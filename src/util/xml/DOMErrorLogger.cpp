/* 
 * "@(#)$Id$"
 */
#include "util/xml/DOMErrorLogger.h"

namespace smsc   {
namespace util   {
namespace xml {

bool DOMErrorLogger::handleError(const DOMError &domError)
  /* throw(ParseException)*/
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
  
  ParseException  exc("%s at file \"%s\" [l:%d, c:%d]\n  Message: %s",
                      s1, uri, domError.getLocation()->getLineNumber(),
                      domError.getLocation()->getColumnNumber(), message);

  _logger->log(logLvl, exc.what());
  
  XMLString::release(&uri);
  XMLString::release(&message);

  if (domError.getSeverity() >= _throwLvl)
    throw exc;
  return false;
}

}
}
}

