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
	ConfigTree * tree = createTree();
	std::ostream *out = new std::ofstream(config_filename);
	writeHeader(*out);
	tree->write(*out, "  ");
	writeFooter(*out);
	out->flush();
	delete out;
	delete tree;
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

Manager::ConfigTree * Manager::createTree()
{
	ConfigTree * result = new ConfigTree("root");

	char * _name;
	
	char * svalue;
	for (strParams.First(); strParams.Next(_name, svalue);)
		result->addParam(_name, ConfigParam::stringType, svalue);

	long lvalue;
	char tmp[33];
	for (longParams.First(); longParams.Next(_name, lvalue);)
	{
		sprintf(tmp, "%i", lvalue);
		result->addParam(_name, ConfigParam::intType, tmp);
	}

	bool bvalue;
	for (boolParams.First(); boolParams.Next(_name, bvalue);)
		result->addParam(_name, ConfigParam::boolType, bvalue ? "true" : "false");

	return result;
}

void Manager::ConfigTree::addParam(const char * const _name,
																	 ConfigParam::types type,
																	 const char * const value)
{
	char *sname =	strdup(_name);
	char *p = strrchr(sname, '.');
	if (p != 0)
	{
		*p = 0;
		ConfigTree* node = createSection(sname);
		node->params.push_back(ConfigParam(strdup(p+1), type, strdup(value)));
	}
	else {
		params.push_back(ConfigParam(strdup(_name), type, strdup(value)));
	}
	free(sname);
}

void Manager::ConfigTree::write(std::ostream &out, std::string prefix)
{
	char * _name;
	ConfigTree *val;
	std::string newPrefix(prefix + "  ");
	for(sections.First(); sections.Next(_name, val);)
	{
		out << prefix << "<section name=\"" << _name << "\">" << std::endl;
		val->write(out, newPrefix);
		out << prefix << "</section>" << std::endl;
	}

	for (size_t i=0; i<params.size(); i++)
	{
		out << prefix << "<param name=\"" << params[i].name << "\" type=\"";
		switch (params[i].type)
		{
		case ConfigParam::boolType:
			out << "bool";
			break;
		case ConfigParam::intType:
			out << "int";
			break;
		case ConfigParam::stringType:
			out << "string";
			break;
		}
		out << "\">" << params[i].value << "</param>" << std::endl;
	}
}

Manager::ConfigTree* Manager::ConfigTree::createSection(const char * const _name)
{
	ConfigTree *t = this;
	char * newName = strdup(_name);
	char * n = newName;
	for (char * p = strchr(n, '.'); p != 0; p = strchr(n, '.'))
	{
		*p=0;
		if (!t->sections.Exists(n))
		{
			t->sections[n] = new ConfigTree(n);
		}
		t = t->sections[n];
		n = p+1;
	}
	if (!t->sections.Exists(n))
	{
		t->sections[n] = new ConfigTree(n);
	}
	return t->sections[n];
}

}
}
}

