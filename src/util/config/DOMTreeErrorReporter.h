#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/sax/ErrorHandler.hpp>
#include <log4cpp/Category.hh>
#include <util/Logger.h>

namespace smsc   {
namespace util   {
namespace config {

/**
 * Внутренний класс, отвечающий за логгирование ошибок в файле конфигурации
 */
class DOMTreeErrorReporter : public ErrorHandler
{
public:
    DOMTreeErrorReporter() :
       fSawErrors(false), logger(smsc::util::Logger::getCategory("smsc.util.config.DomTreeErrorReporter"))
    {
    }

    void warning(const SAXParseException& toCatch);
    void error(const SAXParseException& toCatch);
    void fatalError(const SAXParseException& toCatch);
    void resetErrors();

    bool getSawErrors() const {return fSawErrors;};

private:
    bool    fSawErrors;
	log4cpp::Category& logger;
};

}
}
}

