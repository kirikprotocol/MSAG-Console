#include "Manager.h"
#include "DOMTreeErrorReporter.h"
#include "XmlUtils.h"
#include <xercesc/parsers/DOMParser.hpp>
#include <xercesc/dom/DOM_DOMException.hpp>
#include <xercesc/dom/DOM_NamedNodeMap.hpp>
#include <util/Logger.h>
#include <fstream>

namespace smsc   {
namespace util   {
namespace config {

using std::auto_ptr;

const DOMString db_name(createDOMString("db"));
const DOMString map_name(createDOMString("map"));
const DOMString log_name(createDOMString("log"));
char * Manager::config_filename = 0;
Manager * Manager::manager = 0;

Manager::Manager()
	throw(ConfigException &)
	: logger(smsc::util::Logger::getCategory("smsc.util.config.Manager"))
{
	DOMParser *parser = createParser();

	DOM_Document document = parse(parser, config_filename);
	if (!document.isNull())
	{
		DOM_Element elem = document.getDocumentElement();
		config.parse(elem);
	} else {
		logger.debug("Parse result is null");
	}

//	delete parser->getErrorHandler();
//	delete parser;
}

/**
 * Create our parser, then attach an error handler to the parser.
 * The parser will call back to methods of the ErrorHandler if it
 * discovers errors during the course of parsing the XML document.
 *
 * @return created parser
 */
DOMParser * Manager::createParser() {
	logger.debug("Entering createParser()");
	DOMParser *parser = new DOMParser;
	parser->setValidationScheme(DOMParser::Val_Always);
	parser->setDoNamespaces(false);
	parser->setDoSchema(false);
	parser->setValidationSchemaFullChecking(false);
	logger.debug("  Creating ErrorReporter");
	DOMTreeErrorReporter *errReporter = new DOMTreeErrorReporter();
	parser->setErrorHandler(errReporter);
	parser->setCreateEntityReferenceNodes(false);
	parser->setToCreateXMLDeclTypeNode(false);
	logger.debug("Leaving createParser()");

	return parser;
}

/*!
 * Parse the XML file, catching any XML exceptions that might propogate
 * out of it.
 */
DOM_Document Manager::parse(DOMParser *parser, const char * const filename)
  throw (ConfigException &)
{
	try
	{
		parser->parse(filename);
		int errorCount = parser->getErrorCount();
		if (errorCount > 0) {
			logger.error("An %d errors occured during parsing \"%s\"", errorCount, filename);
			throw new ConfigException("An errors occured during parsing");
		}
	}
	catch (const XMLException& e)
	{
		char * message = DOMString(e.getMessage()).transcode();
		XMLExcepts::Codes code = e.getCode();
		const char *srcFile = e.getSrcFile();
		unsigned int line = e.getSrcLine();
		logger.error("An error occured during parsing \"%s\" at file \"%s\" on line %d. Nested: %d: %s", filename, srcFile, line, code, message);
		delete[] message;
		throw new ConfigException("An errors occured during parsing");
	}
	catch (const DOM_DOMException& e)
	{
		logger.error("A DOM error occured during parsing\"%s\". DOMException code: %i", filename, e.code);
		throw new ConfigException("An errors occured during parsing");
	}
	catch (...)
	{
		logger.error("An error occured during parsing \"%s\"", filename);
		throw new ConfigException("An errors occured during parsing");
	}

	return parser->getDocument();
}


std::ostream & operator << (std::ostream & out, const DOMString & string)
{
	char *p = string.transcode();
	out << p;
	delete[] p;
	return out;
}

/**
 * Записывает конфигурацию системы.
 */
void Manager::save()
{
	std::ostream *out = new std::ofstream(config_filename);
	writeHeader(*out);
	config.save(out);
	writeFooter(*out);
	out->flush();
	delete out;
}

void Manager::writeHeader(std::ostream &out)
{
	out << "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>" << std::endl;
	out << "<!DOCTYPE config SYSTEM \"configuration.dtd\">" << std::endl;
	out << "<config>" << std::endl;
}

void Manager::writeFooter(std::ostream &out)
{
	out << "</config>" << std::endl;
}

}
}
}

