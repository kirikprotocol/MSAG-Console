#ifndef SMSC_UTIL_XML_DOM_TREE_READER
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif /* __GNUC__ */
#define SMSC_UTIL_XML_DOM_TREE_READER

#include <xercesc/dom/DOM.hpp>

#include "core/synchronization/Mutex.hpp"
#include "util/xml/DOMErrorLogger.h"

#if XERCES_VERSION_MAJOR > 2
#include <xercesc/parsers/XercesDOMParser.hpp>
#endif

namespace smsc {
namespace util {
namespace xml {

XERCES_CPP_NAMESPACE_USE
using namespace smsc::core::synchronization;

class DOMTreeReader {
public:
  explicit DOMTreeReader(unsigned throw_lvl = (unsigned)-1);
  ~DOMTreeReader();

  DOMDocument* read(const char* const filename) throw (SmscParseException);

#if XERCES_VERSION_MAJOR > 2
  DOMDocument* read(const InputSource& source) throw (SmscParseException);
#else
  DOMDocument* read(const DOMInputSource& source) throw (SmscParseException);
#endif


protected:

#if XERCES_VERSION_MAJOR > 2
  static XercesDOMParser* createParser(unsigned throw_lvl = (unsigned)-1);
#else
  static DOMBuilder* createParser(unsigned throw_lvl = (unsigned)-1);
#endif


private:
  Mutex     mutex;
  unsigned  _throwLvl;
#if XERCES_VERSION_MAJOR > 2
  std::auto_ptr<XercesDOMParser> parser;
#else
  std::auto_ptr<DOMBuilder> parser;
#endif
};

}
}
}
#endif //ifndef SMSC_UTIL_XML_DOM_TREE_READER

