#ifndef SMSC_UTIL_XML_DOM_TREE_READER
#define SMSC_UTIL_XML_DOM_TREE_READER

#include <xercesc/dom/DOM.hpp>

#include <util/Exception.hpp>
#include <logger/Logger.h>
#include <core/synchronization/Mutex.hpp>

namespace smsc {
namespace util {
namespace xml {

XERCES_CPP_NAMESPACE_USE
using smsc::logger::Logger;
using smsc::util::Exception;
using namespace smsc::core::synchronization;

class ParseException : public Exception
{
public:
  ParseException(const char * const msg) : Exception(msg) {}
};

class DOMTreeReader {
public:

  DOMTreeReader();
  ~DOMTreeReader();

  DOMDocument* read(const char * const filename) throw (ParseException);
  DOMDocument* read(const DOMInputSource & source) throw (ParseException);

protected:
  static DOMBuilder * createParser();
private:
  std::auto_ptr<DOMBuilder> parser;
  Mutex mutex;
};

}
}
}

#endif //ifndef SMSC_UTIL_XML_DOM_TREE_READER
