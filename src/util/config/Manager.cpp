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

const DOMString Manager::db_name(createDOMString("db"));
const DOMString Manager::map_name(createDOMString("map"));
const DOMString Manager::log_name(createDOMString("log"));

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
	if (!document.isNull())
	{
		DOM_Element elem = document.getDocumentElement();
		processTree(elem);
	} else {
		cat.debug("Parse result is null");
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
	if (!element.isNull())
	{
		DOM_NodeList list = element.getChildNodes();
		for (int i=0; i<list.getLength(); i++)
		{
			DOM_Node n = list.item(i);
			if (n.getNodeType() == DOM_Node::ELEMENT_NODE)
			{
				DOM_Element &e = *(DOM_Element*)(&n);
				DOMString name = e.getNodeName();
				if (name.compareString(db_name) == 0)
					db = new Database(e);
				else if (name.compareString(map_name) == 0)
					map = new MapProtocol(e);
				else if (name.compareString(log_name) == 0)
					log = new Log(e);
			}
		}
	}
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
	if (!document.isNull())
	{
		std::ostream *out = new std::ofstream(config_filename);
		writeHeader(*out);
		DOM_Node main_node = document.getDocumentElement();
		writeNode(*out, main_node, 0);
		out->flush();
		delete out;
	}
}

void Manager::writeHeader(std::ostream &out)
{
	out << "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>" << std::endl;
	out << "<!DOCTYPE configuration SYSTEM \"configuration.dtd\">" << std::endl;
}

bool isNodeHasElementChild(const DOM_Node & node)
{
	if (!node.isNull())
	{
		DOM_NodeList list = node.getChildNodes();
		for (unsigned int i=0; i<list.getLength(); i++)
		{
			if (list.item(i).getNodeType() == DOM_Node::ELEMENT_NODE)
			{
				return true;
			}
		}
	}
	return false;
}

void Manager::writeNode(std::ostream &out, DOM_Node & node, unsigned int tabs)
{
	char prefix[tabs+1];
	memset(prefix, '\t', tabs);
	prefix[tabs] = 0;

	out << prefix << '<' << node.getNodeName();
	DOM_NamedNodeMap map = node.getAttributes();
	for (unsigned int i=0; i<map.getLength(); i++)
	{
		DOM_Node attr = map.item(i);
		out << ' ' << attr.getNodeName() << "=\"" << attr.getNodeValue() << '"';
	}
	if (isNodeHasElementChild(node))
	{
		out << '>' << std::endl;
		DOM_NodeList childs = node.getChildNodes();
		for (unsigned int i=0; i<childs.getLength(); i++)
		{
			DOM_Node child = childs.item(i);
			if (child.getNodeType() == DOM_Node::ELEMENT_NODE)
			{
				writeNode(out, child, tabs+1);
			}
		}
		out << prefix << "</" << node.getNodeName() << '>' << std::endl;
	} else {
		out << "/>" << std::endl;
	}
}

}
}
}

