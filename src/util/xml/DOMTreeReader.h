#ifndef SMSC_UTIL_XML_DOM_TREE_READER
#define SMSC_UTIL_XML_DOM_TREE_READER

#include <xercesc/dom/DOM_Document.hpp>
#include <xercesc/parsers/DOMParser.hpp>

#include <util/Exception.hpp>
#include <logger/Logger.h>

namespace smsc {
namespace util {
namespace xml {

using smsc::logger::Logger;
using smsc::util::Exception;

class DOMTreeReader {
public:
	class ParseException : public Exception
	{
	public:
		ParseException(const char * const msg) : Exception(msg) {}
	};

	DOMTreeReader();
	~DOMTreeReader();

	DOM_Document read(const char * const filename) throw (ParseException);
	DOM_Document read(const InputSource & source) throw (ParseException);

protected:
  DOMParser *createParser();
private:
};

}
}
}

#endif //ifndef SMSC_UTIL_XML_DOM_TREE_READER
