#ifndef SMSC_UTIL_XML_DOM_ERROR_LOGGER
#define SMSC_UTIL_XML_DOM_ERROR_LOGGER

#include <xercesc/dom/DOM.hpp>
#include "util/cstrings.h"
//#include <xercesc/dom/DOMErrorHandler.hpp>

namespace smsc {
namespace util {
namespace xml {

XERCES_CPP_NAMESPACE_USE

class DOMErrorLogger : public DOMErrorHandler
{
public:
	DOMErrorLogger();
	DOMErrorLogger(const char * const categoryName);

  virtual bool handleError(const DOMError &domError);

protected:

private:
	std::auto_ptr<char> loggerCategoryName;
};

}
}
}

#endif // ifndef SMSC_UTIL_XML_DOM_ERROR_LOGGER
