#include "Manager.h"
#include "DOMTreeErrorReporter.h"
#include "XmlUtils.h"
#include <xercesc/parsers/DOMParser.hpp>
#include <xercesc/dom/DOM_DOMException.hpp>
#include <util/Logger.h>

namespace smsc   {
namespace util   {
namespace config {

Manager::Manager(const char * const _config_filename)
	throw(ConfigException &)
	: cat(smsc::util::Logger::getCategory("smsc.util.config.Manager"))
{
	document = 0;
	db = 0;
	map = 0;
	log = 0;
	config_filename = _config_filename;

	DOMParser *parser = createParser();

	parse(parser, config_filename);
	processTree(document.getDocumentElement());

	delete parser->getErrorHandler();
	delete parser;
}

/**
 * Записывает конфигурацию системы.
 */
void Manager::save()
{
	//ostream out = new
}

/**
 * Create our parser, then attach an error handler to the parser.
 * The parser will call back to methods of the ErrorHandler if it
 * discovers errors during the course of parsing the XML document.
 *
 * @return created parser
 */
DOMParser * Manager::createParser() {
	cat.debug("Entering createParser()");
	DOMParser *parser = new DOMParser;
	parser->setValidationScheme(DOMParser::Val_Always);
	parser->setDoNamespaces(false);
	parser->setDoSchema(false);
	parser->setValidationSchemaFullChecking(false);
	cat.debug("  Creating ErrorReporter");
	DOMTreeErrorReporter *errReporter = new DOMTreeErrorReporter();
	parser->setErrorHandler(errReporter);
	parser->setCreateEntityReferenceNodes(false);
	parser->setToCreateXMLDeclTypeNode(false);
	cat.debug("Leaving createParser()");

	return parser;
}

/*!
 * Parse the XML file, catching any XML exceptions that might propogate
 * out of it.
 */
void Manager::parse(DOMParser *parser, const char * const filename)
  throw (ConfigException &)
{
	try
	{
		parser->parse(filename);
		int errorCount = parser->getErrorCount();
		if (errorCount > 0) {
			cat.error("An %d errors occured during parsing \"%s\"", errorCount, filename);
			throw new ConfigException("An errors occured during parsing");
		}
	}
	catch (const XMLException& e)
	{
		char * message = DOMString(e.getMessage()).transcode();
		XMLExcepts::Codes code = e.getCode();
		const char *srcFile = e.getSrcFile();
		unsigned int line = e.getSrcLine();
		cat.error("An error occured during parsing \"%s\" at file \"%s\" on line %d. Nested: %d: %s", filename, srcFile, line, code, message);
		delete[] message;
		throw new ConfigException("An errors occured during parsing");
	}
	catch (const DOM_DOMException& e)
	{
		cat.error("A DOM error occured during parsing\"%s\". DOMException code: %i", filename, e.code);
		throw new ConfigException("An errors occured during parsing");
	}
	catch (...)
	{
		cat.error("An error occured during parsing \"%s\"", filename);
		throw new ConfigException("An errors occured during parsing");
	}

	document = parser->getDocument();
}

void Manager::processTree(const DOM_Element &element) {
	DOM_Node tmp = getElementChildByTagName(element, "db");
	db = new Db(tmp);
	tmp = getElementChildByTagName(element, "log");
	log = new Log(tmp);
	tmp = getElementChildByTagName(element, "map");
	map = new Map(tmp);
}

}
}
}

