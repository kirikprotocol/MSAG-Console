#include "DOMErrorLogger.h"
#include <logger/Logger.h>

namespace smsc   {
namespace util   {
namespace xml {

DOMErrorLogger::DOMErrorLogger()
  :loggerCategoryName(cStringCopy("smsc.util.xml.DOMErrorLogger"))
{
}

DOMErrorLogger::DOMErrorLogger(const char * const categoryName)
  :loggerCategoryName(cStringCopy(categoryName))
{
}

bool DOMErrorLogger::handleError(const DOMError &domError)
{
  const char * s1;
  if (domError.getSeverity() == DOMError::DOM_SEVERITY_WARNING)
    s1 = "Warning";
  else if (domError.getSeverity() == DOMError::DOM_SEVERITY_ERROR)
    s1 = "Error";
  else if (domError.getSeverity() == DOMError::DOM_SEVERITY_FATAL_ERROR)
    s1 = "Fatal";
  else
    s1 = "Unknown Fatal";
    
  char *uri = XMLString::transcode(domError.getLocation()->getURI());
  char *message = XMLString::transcode(domError.getMessage());
  
  smsc_log_error(smsc::logger::Logger::getInstance(loggerCategoryName.get()), "%s at file \"%s\" [l:%d, c:%d]\n  Message: %s",
                   s1, uri, 
                   domError.getLocation()->getLineNumber(), domError.getLocation()->getColumnNumber(),
                   message);
  
  XMLString::release(&uri);
  XMLString::release(&message);
  return false;
}

}
}
}

