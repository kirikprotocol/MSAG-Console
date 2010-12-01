#ifndef SMSC_UTIL_XML_DOM_TREE_READER
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif /* __GNUC__ */
#define SMSC_UTIL_XML_DOM_TREE_READER

#include <xercesc/dom/DOM.hpp>

#include "core/synchronization/Mutex.hpp"
#include "util/xml/DOMErrorLogger.h"

namespace smsc {
namespace util {
namespace xml {

XERCES_CPP_NAMESPACE_USE
using namespace smsc::core::synchronization;

class DOMTreeReader {
public:
  explicit DOMTreeReader(unsigned throw_lvl = (unsigned)-1);
  ~DOMTreeReader();

  DOMDocument* read(const char * const filename) throw (ParseException);
  DOMDocument* read(const DOMInputSource & source) throw (ParseException);

protected:
  static DOMBuilder * createParser(unsigned throw_lvl = (unsigned)-1);

private:
  Mutex     mutex;
  unsigned  _throwLvl;
  std::auto_ptr<DOMBuilder> parser;
};

}
}
}
#endif //ifndef SMSC_UTIL_XML_DOM_TREE_READER

