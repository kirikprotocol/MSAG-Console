#include <resourcemanager/LocaleResources.hpp>
#include <string>
#include <util/xml/DOMTreeReader.h>
#include <util/debug.h>
#include <util/xml/utilFunctions.h>

namespace smsc {
namespace resourcemanager {

using smsc::util::xml::DOMTreeReader;

void LocaleResources::processParams(const DOM_Element &elem, LocaleResources::_stringmap & settings, const std::string &prefix)
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

LocaleResources::LocaleResources(const std::string & filename)
{
	__trace2__("Load locale resources from \"%s\"\n", filename.c_str());
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
	__trace2__("Locale resources from \"%s\" sucessfully loaded (%u settings, %u resources)\n", filename.c_str(), settings.size(), resources.size());
}

// возвращает строку из сетингов для ключа.
std::string LocaleResources::getSetting(const std::string & key)
{
	return settings[key];
}

// возвращает строку из ресурса для ключа.
std::string LocaleResources::getString(const std::string & key)
{
	return resources[key];
}

#ifdef SMSC_DEBUG
void LocaleResources::dump(std::ostream & outStream)
{
	outStream << "  Settings:" << std::endl;
	for (_stringmap::iterator i = settings.begin(); i!=settings.end(); i++)
	{
		outStream << "    " << i->first << ":" << i->second << std::endl;
	}

	outStream << "  Resources:" << std::endl;
	for (_stringmap::iterator i = resources.begin(); i!=resources.end(); i++)
	{
		outStream << "    " << i->first << ":" << i->second << std::endl;
	}
}
#endif //ifdef SMSC_DEBUG

}
}
