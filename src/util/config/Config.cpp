#include "Config.h"

#include <xercesc/dom/DOM_NodeList.hpp>
#include <memory>
#include <iostream>

#include <util/xml/utilFunctions.h>

namespace smsc {
namespace util {
namespace config {

using std::auto_ptr;
using smsc::util::xml::getNodeText;

void Config::parse(const DOM_Element &element)
	throw (ConfigException)
{
	try {
    processNode(element,"");	
	}
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

void Config::processNode(const DOM_Element &element,
												 const char * const prefix)
	throw (DOM_DOMException)
{
	if (!element.isNull())
	{
		DOM_NodeList list = element.getChildNodes();
		for (unsigned i=0; i<list.getLength(); i++)
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
					log4cpp::Category &logger(Logger::getCategory("smsc.util.config.Config"));
					logger.warn("Unknown node \"%s\" in section \"%s\"", nodeName.get(), prefix);
				}
			}
		}
	}
}

void Config::processParamNode(const DOM_Element &element,
															const char * const name,
															const char * const type)
	throw (DOM_DOMException)
{
	//getting value
	std::auto_ptr<char> value(getNodeText(element));
	
	if (strcmp(type, "string") == 0)
	{
		setString(name, value.get());
	}
	else if (strcmp(type, "int") == 0)
	{
		setInt(name, strtol(value.get(), 0, 0));
	}
	else if (strcmp(type, "bool") == 0)
	{
		if ((strcmp(value.get(), "true") == 0)
				|| (strcmp(value.get(), "on") == 0)
				|| (strcmp(value.get(), "yes") == 0))
		{
			setBool(name, true);
		}
		else if ((strcmp(value.get(), "false") == 0)
						 || (strcmp(value.get(), "off") == 0)
						 || (strcmp(value.get(), "no") == 0))
		{
			setBool(name, false);
		}
		else
		{
			log4cpp::Category &logger(Logger::getCategory("smsc.util.config.Config"));
			logger.warn("Unrecognized boolean value \"%s\" for parameter \"%s\". Setted to FALSE.", value.get(), name);
			setBool(name, false);
		}
	}
	else
	{
		log4cpp::Category &logger(Logger::getCategory("smsc.util.config.Config"));
		logger.warn("Unrecognized parameter type \"%s\" for parameter \"%s\"", type, name);
	}
}

Config::ConfigTree * Config::createTree() const
{
	ConfigTree * result = new ConfigTree("root");

	char * _name;
	
	char * svalue;
	for (Hash<char *>::Iterator i= strParams.getIterator(); i.Next(_name, svalue);)
		result->addParam(_name, ConfigParam::stringType, svalue);

	int32_t lvalue;
	char tmp[33];
	for (Hash<int32_t>::Iterator i = intParams.getIterator(); i.Next(_name, lvalue);)
	{
		snprintf(tmp, sizeof(tmp), "%i", lvalue);
		result->addParam(_name, ConfigParam::intType, tmp);
	}

	bool bvalue;
	for (Hash<bool>::Iterator i = boolParams.getIterator(); i.Next(_name, bvalue);)
		result->addParam(_name, ConfigParam::boolType, bvalue ? "true" : "false");

	return result;
}

void Config::ConfigTree::addParam(const char * const _name,
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

void Config::ConfigTree::write(std::ostream &out, std::string prefix)
{
	char * _name;
	ConfigTree *val;
	std::string newPrefix(prefix + "  ");
	for(sections.First(); sections.Next(_name, val);)
	{
		std::auto_ptr<char> tmp_name(encode(_name));
		out << prefix << "<section name=\"" << tmp_name.get() << "\">" << std::endl;
		val->write(out, newPrefix);
		out << prefix << "</section>" << std::endl;
	}

	for (size_t i=0; i<params.size(); i++)
	{
		std::auto_ptr<char> paramName(encode(params[i].name));
		std::auto_ptr<char> paramValue(encode(params[i].value));
		out << prefix << "<param name=\"" << paramName.get() << "\" type=\"";
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
		out << "\">" << paramValue.get() << "</param>" << std::endl;
	}
}

char * Config::ConfigTree::getText(std::string prefix)
{
	std::string result;
	char * _name;
	ConfigTree *val;
	std::string newPrefix(prefix + "  ");
	for(sections.First(); sections.Next(_name, val);)
	{
		std::auto_ptr<char> tmp_name(encode(_name));
		result += prefix;
		result += "<section name=\"";
		result += tmp_name.get();
		result += "\">\n";
		char * tmp = val->getText(newPrefix);
		result += tmp;
		delete[] tmp;
		result += prefix;
		result += "</section>\n";
	}

	for (size_t i=0; i<params.size(); i++)
	{
		std::auto_ptr<char> paramName(encode(params[i].name));
		std::auto_ptr<char> paramValue(encode(params[i].value));
		result += prefix;
		result += "<param name=\"";
		result += paramName.get();
		result += "\" type=\"";
		switch (params[i].type)
		{
		case ConfigParam::boolType:
			result += "bool";
			break;
		case ConfigParam::intType:
			result += "int";
			break;
		case ConfigParam::stringType:
			result += "string";
			break;
		}
		result += "\">";
		result += paramValue.get();
		result += "</param>\n";
	}
	char * str_result = new char[result.size() +1];
	strcpy(str_result, result.c_str());
	return str_result;
}

Config::ConfigTree* Config::ConfigTree::createSection(const char * const _name)
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

template<class _HashType, class _HashIteratorType, class _ValueType>
void removeSectionFromHash(_HashType &hash,
													 const char * const sectionName,
													 size_t sectionNameLen)
{
	char *name;
	_ValueType value;
	for (_HashIteratorType i = hash.getIterator(); i.Next(name, value); )
	{
		if (strlen(name) >= sectionNameLen
				&& name[sectionNameLen] == '.'
				&& memcmp(name, sectionName, sectionNameLen) == 0)
		{
			hash.Delete(name);
		}
	}
}

void Config::removeSection(const char * const sectionName)
{
	const size_t sectionNameLen = strlen(sectionName);

	removeSectionFromHash<intParamsType, intParamsType::Iterator, int32_t>
		(intParams, sectionName, sectionNameLen);
	
	removeSectionFromHash<boolParamsType, boolParamsType::Iterator, bool>
		(boolParams, sectionName, sectionNameLen);

	removeSectionFromHash<strParamsType, strParamsType::Iterator, char *>
		(strParams, sectionName, sectionNameLen);
}

void collect_section_names_into_set(CStrSet &result,
																		const char * const sectionName,
																		const size_t sectionNameLen,
																		const char * const name)
{
	if (strlen(name) >= sectionNameLen
			&& name[sectionNameLen] == '.'
			&& memcmp(name, sectionName, sectionNameLen) == 0)
	{
		char* dotpos = strchr(name+sectionNameLen+1, '.');
		if (dotpos != 0)
		{
			char sectName[dotpos - name+1];
			memcpy(sectName, name, dotpos - name);
      sectName[dotpos - name] = 0;
			result.insert(std::string(sectName));
		}
	}
}

template<class _HashType, class _HashIteratorType, class _ValueType>
void getChildSectionsFromHash(_HashType &hash,
															const char * const sectionName,
															size_t sectionNameLen,
															CStrSet &result)
{
	char *name;
	_ValueType value;
	for (_HashIteratorType i = hash.getIterator(); i.Next(name, value); )
	{
		collect_section_names_into_set(result, sectionName, sectionNameLen, name);
	}
}

template<class _HashType, class _HashIteratorType, class _ValueType>
void getChildParamsFromHash(_HashType &hash,
                            const char * const sectionName,
														size_t sectionNameLen,
														CStrSet &result)
{
	char *name;
	_ValueType value;
  for (_HashIteratorType i = hash.getIterator(); i.Next(name, value); ) 
  {
    if (memcmp(name, sectionName, sectionNameLen) == 0) 
    {
      char paramName[strlen(name) -(sectionNameLen +1) +1];
			strcpy(paramName, name +(sectionNameLen +1));
      result.insert(std::string(paramName));
    }
  }
}

CStrSet* Config::getChildSectionNames(const char * const sectionName)
{
	CStrSet *result = new CStrSet;
	const size_t sectionNameLen = sectionName[strlen(sectionName)-1] == '.'
		? strlen(sectionName)-1
		: strlen(sectionName);
	
	getChildSectionsFromHash<intParamsType, intParamsType::Iterator, int32_t>
		(intParams, sectionName, sectionNameLen, *result);

	getChildSectionsFromHash<boolParamsType, boolParamsType::Iterator, bool>
		(boolParams, sectionName, sectionNameLen, *result);
	
	getChildSectionsFromHash<strParamsType, strParamsType::Iterator, char *>
		(strParams, sectionName, sectionNameLen, *result);
	
	return result;
}

CStrSet * Config::getChildIntParamNames(const char * const sectionName)
{
  std::auto_ptr<CStrSet> result(new CStrSet);
	const size_t sectionNameLen = sectionName[strlen(sectionName)-1] == '.'
		? strlen(sectionName)-1
		: strlen(sectionName);
	
	getChildParamsFromHash<intParamsType, intParamsType::Iterator, int32_t>
		(intParams, sectionName, sectionNameLen, *result);

  return result.release();
}

CStrSet * Config::getChildBoolParamNames(const char * const sectionName)
{
  std::auto_ptr<CStrSet> result(new CStrSet);
	const size_t sectionNameLen = sectionName[strlen(sectionName)-1] == '.'
		? strlen(sectionName)-1
		: strlen(sectionName);
	
	getChildParamsFromHash<boolParamsType, boolParamsType::Iterator, bool>
		(boolParams, sectionName, sectionNameLen, *result);

  return result.release();
}

CStrSet * Config::getChildStrParamNames(const char * const sectionName)
{
  std::auto_ptr<CStrSet> result(new CStrSet);
	const size_t sectionNameLen = sectionName[strlen(sectionName)-1] == '.'
		? strlen(sectionName)-1
		: strlen(sectionName);

	getChildParamsFromHash<strParamsType, strParamsType::Iterator, char *>
		(strParams, sectionName, sectionNameLen, *result);
	
	return result.release();
}

}
}
}

