#ifndef SMSC_UTIL_XML_DOM_TREE_READER
#define SMSC_UTIL_XML_DOM_TREE_READER

#include <log4cpp/Category.hh>
#include <xercesc/dom/DOM_Document.hpp>
#include <xercesc/parsers/DOMParser.hpp>

namespace smsc {
namespace util {
namespace xml {

using log4cpp::Category;

class DOMTreeReader {
public:
	class ParseException : public std::exception
	{
	public:
		ParseException(const char * const message)
		{
			mess = new char[strlen(message) +1];
			strcpy(mess, message);
		}

		virtual ~ParseException() throw()
		{
			delete[] mess;
		}

		virtual const char* what() const throw()
		{
			return mess;
		}
	private:
		char *mess;
	};

	DOMTreeReader();
	~DOMTreeReader();

	DOM_Document read(const char * const filename) throw (ParseException &);

protected:
	log4cpp::Category &logger;
	DOMParser *parser;
private:
};

}
}
}

#endif //ifndef SMSC_UTIL_XML_DOM_TREE_READER
