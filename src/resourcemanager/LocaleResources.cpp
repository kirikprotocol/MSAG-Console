#include <resourcemanager/LocaleResources.hpp>
#include <string>
#include <util/xml/DOMTreeReader.h>
#include <util/debug.h>
#include <util/xml/utilFunctions.h>
#include <util/Logger.h>
#include <xercesc/dom/DOM_DOMException.hpp>
#include <xercesc/sax/SAXException.hpp>

namespace smsc {
namespace resourcemanager {

using smsc::util::xml::DOMTreeReader;

void LocaleResources::processParams(const DOM_Element &elem, LocaleResources::_stringmap & settings, const std::string &prefix) throw ()
{
	log4cpp::Category &logger(smsc::util::Logger::getCategory("smsc.resourcemanager.LocaleResources"));
	try 
	{
		DOM_NodeList sectionList = elem.getChildNodes();
		for (int i = 0; i < sectionList.getLength(); i++)
		{
			DOM_Node node = sectionList.item(i);
			if (node.getNodeType() == DOM_Node::ELEMENT_NODE)
			{
				DOM_Element &elem = *(DOM_Element*)(&node);
				std::auto_ptr<char> elemName(elem.getNodeName().transcode());
				if (strcmp(elemName.get(), "section") == 0)
				{
					std::auto_ptr<char> sectionName(elem.getAttribute("name").transcode());
					std::string newPrefix = prefix;
					if (!prefix.empty())
						newPrefix += '.';
					newPrefix += sectionName.get();
					processParams(elem, settings, newPrefix);
				}
				else if (strcmp(elemName.get(), "param") == 0)
				{
					std::auto_ptr<char> name(elem.getAttribute("name").transcode());
					std::auto_ptr<char> value(smsc::util::xml::getNodeText(elem));
					std::string paramName = prefix;
					if (!prefix.empty())
						paramName += '.';
					paramName += name.get();
					settings[paramName] = value.get();
				}
			}
		}
	}
	catch (smsc::util::Exception &e)
	{
		logger.error("Couldn't load locale: %s", e.what());
	}
	catch (std::exception &e)
	{
		logger.error("Couldn't load locale: %s", e.what());
	}
	catch (...)
	{
		logger.error("Couldn't load locale: Unknown exception in processParam");
	}
}

LocaleResources::LocaleResources(const std::string & filename) throw ()
{
	__trace2__("Load locale resources from \"%s\"\n", filename.c_str());
	log4cpp::Category &logger(smsc::util::Logger::getCategory("smsc.resourcemanager.LocaleResources"));
	try
	{
		DOMTreeReader reader;
		DOM_Element doc = reader.read(filename.c_str()).getDocumentElement();
		
		DOM_NodeList settingsList = doc.getElementsByTagName("settings");
		for (unsigned i = 0; i < settingsList.getLength(); i++)
		{
			DOM_Node node = settingsList.item(i);
			processParams(*(DOM_Element*)(&node), settings, "");
		}
	
		DOM_NodeList recourcesList = doc.getElementsByTagName("resources");
		for (unsigned i = 0; i < recourcesList.getLength(); i++)
		{
			DOM_Node node = recourcesList.item(i);
			processParams(*(DOM_Element*)(&node), resources, "");
		}
	}
	catch (const XMLException& e)
	{
		std::auto_ptr<char> message(DOMString(e.getMessage()).transcode());
		XMLExcepts::Codes code = e.getCode();
		const char *srcFile = e.getSrcFile();
		unsigned int line = e.getSrcLine();
		logger.error("Couldn't construct locale \"%s\": An error occured during parsing received (\"%s\") command on line %d. Nested: %d: %s", 
								 filename.c_str(), srcFile, line, code, message.get());
		__trace2__("Locale resources from \"%s\" not loaded due to error: %s\n", filename.c_str(), message.get());
		return;
	}
	catch (const DOM_DOMException& e)
	{
		logger.error("Couldn't construct locale \"%s\": DOMException code %i", filename.c_str(), e.code);
		__trace2__("Locale resources from \"%s\" not loaded due to error: DOMException code %i\n", filename.c_str(), e.code);
		return;
	}
	catch (const SAXException &e)
	{
		std::auto_ptr<char> message(DOMString(e.getMessage()).transcode());
		logger.error("Couldn't construct locale \"%s\": %s", filename.c_str(), message.get());
		__trace2__("Locale resources from \"%s\" not loaded due to error: %s\n", filename.c_str(), message.get());
		return;
	}
	catch (smsc::util::Exception &e)
	{
		logger.error("Couldn't construct locale \"%s\": %s", filename.c_str(), e.what());
		__trace2__("Locale resources from \"%s\" not loaded due to error: %s\n", filename.c_str(), e.what());
		return;
	}
	catch (std::exception &e)
	{
		logger.error("Couldn't construct locale \"%s\": %s", filename.c_str(), e.what());
		__trace2__("Locale resources from \"%s\" not loaded due to error: %s\n", filename.c_str(), e.what());
		return;
	}
	catch (...)
	{
		logger.error("Couldn't construct locale \"%s\": Unknown exception in constructor", filename.c_str());
		__trace2__("Locale resources from \"%s\" not loaded due to errors\n", filename.c_str());
		return;
	}
	__trace2__("Locale resources from \"%s\" sucessfully loaded (%u settings, %u resources)\n", filename.c_str(), settings.size(), resources.size());
}

// возвращает строку из сетингов для ключа.
std::string LocaleResources::getSetting(const std::string & key) const throw ()
{
	const _stringmap::const_iterator & value = settings.find(key);
	if (value != settings.end())
		return value->second;
	else
		return "";
}

// возвращает строку из ресурса для ключа.
std::string LocaleResources::getString(const std::string & key) const throw ()
{
	const _stringmap::const_iterator & value = resources.find(key);
	if (value != resources.end())
		return value->second;
	else
		return "";
}

#ifdef SMSC_DEBUG
void LocaleResources::dump(std::ostream & outStream) const
{
	outStream << "  Settings:" << std::endl;
	for (_stringmap::const_iterator i = settings.begin(); i!=settings.end(); i++)
	{
		outStream << "    " << i->first << ":" << i->second << std::endl;
	}

	outStream << "  Resources:" << std::endl;
	for (_stringmap::const_iterator i = resources.begin(); i!=resources.end(); i++)
	{
		outStream << "    " << i->first << ":" << i->second << std::endl;
	}
}
#endif //ifdef SMSC_DEBUG

}
}
