#include <resourcemanager/LocaleResources.hpp>
#include <string>
#include <util/xml/DOMTreeReader.h>
#include <util/debug.h>
#include <util/xml/utilFunctions.h>
#include <logger/Logger.h>
#include <xercesc/dom/DOM_DOMException.hpp>
#include <xercesc/sax/SAXException.hpp>
#include "util/debug.h"

namespace smsc {
namespace resourcemanager {

using smsc::util::xml::DOMTreeReader;

std::string strToLower(const std::string & str)
{
	char dst[str.length()+1];
	const char * const src = str.c_str();
	const size_t length	= str.length();
	for (int i=0; i<length; i++)
		dst[i] = tolower(src[i]);
	dst[length] = 0;
	return dst;
}

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
          settings[strToLower(paramName)] = value.get();
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

LocaleResources::~LocaleResources()
{
  std::map<std::string,OutputFormatter*>::iterator i=formatters.begin();
  while(i!=formatters.end())
  {
    delete i->second;
    i++;
  }
}


// возвращает строку из сетингов для ключа.
std::string LocaleResources::getSetting(const std::string & key) const throw ()
{
  const _stringmap::const_iterator & value = settings.find(strToLower(key));
  if (value != settings.end())
    return value->second;
  else
    return "";
}

// возвращает строку из ресурса для ключа.
std::string LocaleResources::getString(const std::string & key) const throw ()
{
  const _stringmap::const_iterator & value = resources.find(strToLower(key));
  if (value != resources.end())
  {
    __trace2__("LR: key=%s, value=%s",key.c_str(),value->second.c_str());
    return value->second;
  }
  else
  {
    __trace2__("LR: string for key=%s not found",key.c_str());
    return "";
  }
}

bool LocaleResources::hasString(const std::string& key)const throw()
{
  return resources.find(strToLower(key))!=resources.end();
}


OutputFormatter* LocaleResources::getFormatter(const std::string& key)throw()
{
	std::string lower_key(strToLower(key));
  std::map<std::string,OutputFormatter*>::const_iterator i=formatters.find(lower_key);
  if(i!=formatters.end())
  {
    __trace2__("LR: found cached formatter for key %s",lower_key.c_str());
    return i->second;
  }
  std::string str=getString(lower_key).c_str();
  __trace2__("LR: creating new formatter for key %s, text=%s",lower_key.c_str(),str.c_str());

  OutputFormatter* res=new OutputFormatter(str.c_str());
  __trace2__("LR: formatter=%p",res);
  formatters.insert(std::pair<std::string,OutputFormatter*>(lower_key,res));
  return res;
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
