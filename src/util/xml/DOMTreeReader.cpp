#include "DOMTreeReader.h"

#include <util/Logger.h>
#include <util/xml/DOMErrorLogger.h>
#include <util/xml/init.h>
#include <xercesc/dom/DOM_DOMException.hpp>
#include <xercesc/sax/SAXException.hpp>

namespace smsc {
namespace util {
namespace xml {

using smsc::util::Logger;

DOMTreeReader::DOMTreeReader()
	: logger(Logger::getCategory("smsc.util.xml.DOMTreeReader"))
{
	initXerces();
	logger.debug("Creating parser");
	parser = new DOMParser;
	parser->setValidationScheme(DOMParser::Val_Always);
	parser->setDoNamespaces(false);
	parser->setDoSchema(false);
	parser->setValidationSchemaFullChecking(false);
	logger.debug("  Creating ErrorReporter");
	DOMErrorLogger *errReporter = new DOMErrorLogger("smsc.util.xml.DOMTreeReader");
	parser->setErrorHandler(errReporter);
	parser->setCreateEntityReferenceNodes(false);
	parser->setToCreateXMLDeclTypeNode(false);
	logger.debug("Parser created");
}

DOMTreeReader::~DOMTreeReader()
{
	delete parser;
}

DOM_Document DOMTreeReader::read(const char * const filename)
	throw (ParseException &)
{
	try
	{
		parser->reset();
		parser->parse(filename);
		int errorCount = parser->getErrorCount();
		if (errorCount > 0) {
			logger.error("An %d errors occured during parsing \"%s\"", errorCount, filename);
			throw new ParseException("An errors occured during parsing");
		}
	}
	catch (const XMLException& e)
	{
		std::auto_ptr<char> message(DOMString(e.getMessage()).transcode());
		XMLExcepts::Codes code = e.getCode();
		const char *srcFile = e.getSrcFile();
		unsigned int line = e.getSrcLine();
		logger.error("An error occured during parsing \"%s\" at file \"%s\" on line %d. Nested: %d: %s", filename, srcFile, line, code, message.get());
		throw new ParseException(message.get());
	}
	catch (const DOM_DOMException& e)
	{
		logger.error("A DOM error occured during parsing\"%s\". DOMException code: %i", filename, e.code);
		throw new ParseException("An errors occured during parsing");
	}
	catch (const SAXException &e)
	{
		std::auto_ptr<char> message(DOMString(e.getMessage()).transcode());
		logger.error("A DOM error occured during parsing\"%s\", nested: %s", filename, message.get());
		throw new ParseException(message.get());
	}
	catch (...)
	{
		logger.error("An error occured during parsing \"%s\"", filename);
		throw new ParseException("An errors occured during parsing");
	}

	return parser->getDocument();
}

}
}
}


