#ifndef SMSC_UTIL_XML_DOM_ERROR_LOGGER
#define SMSC_UTIL_XML_DOM_ERROR_LOGGER

#include <xercesc/sax/ErrorHandler.hpp>
#include <log4cpp/Category.hh>
#include <util/Logger.h>

namespace smsc {
namespace util {
namespace xml {

class DOMErrorLogger : public ErrorHandler
{
public:
	DOMErrorLogger()
		: fSawErrors(false),
		  logger(smsc::util::Logger::getCategory("smsc.util.xml.DOMErrorLogger"))
	{}

	DOMErrorLogger(const char * const categoryName)
		: fSawErrors(false),
		  logger(smsc::util::Logger::getCategory(categoryName))
	{}

	void warning(const SAXParseException& toCatch);
	void error(const SAXParseException& toCatch);
	void fatalError(const SAXParseException& toCatch);
	void resetErrors();

	bool getSawErrors() const {return fSawErrors;};

protected:

private:
	bool fSawErrors;
	log4cpp::Category& logger;
};

}
}
}

#endif // ifndef SMSC_UTIL_XML_DOM_ERROR_LOGGER
