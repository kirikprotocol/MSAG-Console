#ifndef SMSC_UTIL_XML_DOM_TREE_READER
#define SMSC_UTIL_XML_DOM_TREE_READER

#include <log4cpp/Category.hh>
#include <xercesc/dom/DOM_Document.hpp>
#include <xercesc/parsers/DOMParser.hpp>

#include <util/Exception.hpp>

namespace smsc {
namespace util {
namespace xml {

using log4cpp::Category;
using	smsc::util::Exception;

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

protected:
	DOMParser *parser;
private:
};

}
}
}

#endif //ifndef SMSC_UTIL_XML_DOM_TREE_READER
