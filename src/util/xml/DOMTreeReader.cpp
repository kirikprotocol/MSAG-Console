#include "DOMTreeReader.h"

#include <util/Logger.h>
#include <util/xml/DOMErrorLogger.h>
#include <util/xml/init.h>
#include <util/xml/DtdResolver.h>
#include <xercesc/dom/DOM_DOMException.hpp>
#include <xercesc/sax/SAXException.hpp>

namespace smsc {
namespace util {
namespace xml {

using smsc::util::Logger;

DOMTreeReader::DOMTreeReader()
{
	initXerces();
	parser = new DOMParser;
	parser->setValidationScheme(DOMParser::Val_Always);
	parser->setDoNamespaces(false);
	parser->setDoSchema(false);
	parser->setValidationSchemaFullChecking(false);
	parser->setErrorHandler(new DOMErrorLogger("smsc.util.xml.DOMTreeReader"));
	parser->setCreateEntityReferenceNodes(false);
	parser->setToCreateXMLDeclTypeNode(false);
	parser->setEntityResolver(new DtdResolver());
}

DOMTreeReader::~DOMTreeReader()
{
	delete parser;
}

DOM_Document DOMTreeReader::read(const char * const filename)
	throw (ParseException)
{
	LocalFileInputSource source(DOMString(filename).rawBuffer());
	return read(source);
}

DOM_Document DOMTreeReader::read(const InputSource & source)
	throw (ParseException)
{
	try
	{
		parser->reset();
		parser->parse(source);
		int errorCount = parser->getErrorCount();
		if (errorCount > 0) {
			log4cpp::Category &logger(Logger::getCategory("smsc.util.xml.DOMTreeReader"));
			logger.error("An %d errors occured during parsing received command", errorCount);
			throw ParseException("An errors occured during parsing");
		}
	}
	catch (const XMLException& e)
	{
		std::auto_ptr<char> message(DOMString(e.getMessage()).transcode());
		XMLExcepts::Codes code = e.getCode();
		const char *srcFile = e.getSrcFile();
		unsigned int line = e.getSrcLine();
		log4cpp::Category &logger(Logger::getCategory("smsc.util.xml.DOMTreeReader"));
		logger.error("An error occured during parsing received (\"%s\") command on line %d. Nested: %d: %s", srcFile, line, code, message.get());
		throw ParseException(message.get());
	}
	catch (const DOM_DOMException& e)
	{
		log4cpp::Category &logger(Logger::getCategory("smsc.util.xml.DOMTreeReader"));
		logger.error("A DOM error occured during parsing received command. DOMException code: %i", e.code);
		throw ParseException("An errors occured during parsing");
	}
	catch (const SAXException &e)
	{
		std::auto_ptr<char> message(DOMString(e.getMessage()).transcode());
		log4cpp::Category &logger(Logger::getCategory("smsc.util.xml.DOMTreeReader"));
		logger.error("A DOM error occured during parsing received command, nested: %s", message.get());
		throw ParseException(message.get());
	}
	catch (...)
	{
		log4cpp::Category &logger(Logger::getCategory("smsc.util.xml.DOMTreeReader"));
		logger.error("An error occured during parsing  received command");
		throw ParseException("An errors occured during parsing");
	}

	return parser->getDocument();
}

}
}
}


