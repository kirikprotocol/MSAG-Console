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
		processTree(elem);

		char * key;
		char * value;
		for (strParams.First(); strParams.Next(key, value);)
		{
			std::cout << key << ": " << value << "\n\r";
		}

		long ivalue;
		for (strParams.First(); longParams.Next(key, ivalue);)
		{
			std::cout << key << ": " << ivalue << "\n\r";
		}

		bool bvalue;
		for (strParams.First(); boolParams.Next(key, bvalue);)
		{
			std::cout << key << ": " << bvalue << "\n\r";
		}
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

void Manager::processTree(const DOM_Element &element)
	throw (ConfigException &)
{
	try {
		processNode(element,"");	}
	catch (DOM_DOMException &e)
	{
		std::string s("Exception on processing config tree, nested: ");
		char *msg = e.msg.transcode();
		s += msg;
		delete[] msg;
		throw ConfigException(s.c_str());
	}
	catch (...)
	{
		throw ConfigException("Exception on processing config tree");
	}
}

void Manager::processNode(const DOM_Element &element,
													const char * const prefix)
	throw (DOM_DOMException &)
{
	if (!element.isNull())
	{
		DOM_NodeList list = element.getChildNodes();
		for (int i=0; i<list.getLength(); i++)
		{
			DOM_Node n = list.item(i);
			if (n.getNodeType() == DOM_Node::ELEMENT_NODE)
			{
				DOM_Element &e = *(DOM_Element*)(&n);
				auto_ptr<const char> name(e.getAttribute("name").transcode());
				auto_ptr<char> fullName(new char[strlen(prefix) +1 +strlen(name.get()) +1]);
				if (prefix[0] != 0)
				{
					strcpy(fullName.get(), prefix);
					strcat(fullName.get(), ".");
					strcat(fullName.get(), name.get());
				} else {
					strcpy(fullName.get(), name.get());
				}
				auto_ptr<const char> nodeName(e.getNodeName().transcode());
				if (strcmp(nodeName.get(), "section") == 0)
				{
					processNode(e, fullName.get());
				}
				else if (strcmp(nodeName.get(), "param") == 0)
				{
					auto_ptr<const char> type(e.getAttribute("type").transcode());
					processParamNode(e, fullName.release(), type.get());
				}
				else
				{
					logger.warn("Unknown node \"%s\" in section \"%s\"", nodeName.get(), prefix);
				}
			}
		}
	}
}

void Manager::processParamNode(const DOM_Element &element,
															 const char * const name,
															 const char * const type)
	throw (DOM_DOMException &)
{
	//getting value
	std::string result;
	if (!element.isNull())
	{
		DOM_NodeList list = element.getChildNodes();
		for (int i=0; i<list.getLength(); i++)
		{
			DOM_Node n = list.item(i);
			if (n.getNodeType() == DOM_Node::TEXT_NODE)
			{
				const char * const value = n.getNodeValue().transcode();
				result += value;
				delete[] value;
			}
		}
	}
	
	const size_t value_len = result.length();
	auto_ptr<char> value(new char[value_len +1]);
	strcpy(value.get(), result.c_str());
	if (strcmp(type, "string") == 0)
	{
		strParams[name] = value.release();
	}
	else if (strcmp(type, "int") == 0)
	{
		char * p = value.get() + value_len;
		longParams[name] = strtol(value.get(), &p, 0);
	}
	else if (strcmp(type, "bool") == 0)
	{
		if ((strcmp(value.get(), "true") == 0)
				|| (strcmp(value.get(), "on") == 0)
				|| (strcmp(value.get(), "yes") == 0))
		{
			boolParams[name] = true;
		}
		else if ((strcmp(value.get(), "false") == 0)
						 || (strcmp(value.get(), "off") == 0)
						 || (strcmp(value.get(), "no") == 0))
		{
			boolParams[name] = false;
		}
		else
		{
			logger.warn("Unrecognized boolean value \"%s\" for parameter \"%s\"", value.get(), name);
		}
	}
	else
	{
		logger.warn("Unrecognized parameter type \"%s\" for parameter \"%s\"", type, name);
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
/*	if (!document.isNull())
	{
		std::ostream *out = new std::ofstream(config_filename);
		writeHeader(*out);
		//writeNode(*out, main_node, 0);
		out->flush();
		delete out;
	}*/
}

void Manager::writeHeader(std::ostream &out)
{
	out << "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>" << std::endl;
	out << "<!DOCTYPE config SYSTEM \"configuration.dtd\">" << std::endl;
	out << "<config>" << std::endl;
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

