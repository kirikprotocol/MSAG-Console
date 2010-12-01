#ifndef SMSC_UTIL_XML_DOM_ERROR_LOGGER
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif /* __GNUC__ */
#define SMSC_UTIL_XML_DOM_ERROR_LOGGER

#include <memory>
#include <xercesc/dom/DOM.hpp>
//#include <xercesc/dom/DOMErrorHandler.hpp>

#include "util/Exception.hpp"
#include "logger/Logger.h"

namespace smsc {
namespace util {
namespace xml {

using smsc::logger::Logger;

XERCES_CPP_NAMESPACE_USE

class ParseException : public smsc::util::Exception {
public:
  ParseException(const char * const fmt, ...)  : smsc::util::Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};


class DOMErrorLogger : public DOMErrorHandler {
private:
  unsigned  _throwLvl;
  Logger *  _logger;

public:
  explicit DOMErrorLogger(unsigned throw_lvl = (unsigned)-1,
                          const char * const log_category = NULL)
    : _throwLvl(throw_lvl)
    , _logger(Logger::getInstance(log_category ? log_category
                                  : "smsc.util.xml.DOMErrorLogger"))
  { }
  ~DOMErrorLogger()
  { }

  void setThrowLevel(DOMError::ErrorSeverity dom_err_lvl)
  {
    _throwLvl = dom_err_lvl;
  }
  void clearThrowLevel(void)
  {
    _throwLvl = (unsigned)-1;
  }

  virtual bool handleError(const DOMError &domError)
    /* throw(ParseException)*/;
};

}
}
}
#endif // ifndef SMSC_UTIL_XML_DOM_ERROR_LOGGER

