#include <resourcemanager/ResourceManager.hpp>
#include <string>
#include <list>
#include <iostream>
#include <dirent.h>
#include <util/Exception.hpp>
#include <util/Logger.h>
#include "util/debug.h"

namespace smsc{
namespace resourcemanager{

using smsc::util::Exception;
using namespace smsc::core::synchronization;

std::auto_ptr<ResourceManager> ResourceManager::instance(0);
std::string ResourceManager::defaultLocale("en_en");
Mutex ResourceManager::mtx;
OutputFormatter nullFmt("");

const ResourceManager* ResourceManager::getInstance() throw()
{
  MutexGuard g(mtx);
  if (instance.get() == 0)
  {
    instance.reset(new ResourceManager);
  }
  return instance.get();
}

void ResourceManager::init(const char * const localesString, const char * const defaultLocaleStr) throw()
{
  std::string defaultLocale(defaultLocaleStr);
  std::list<std::string> locales;
  const char * str = localesString;
  for (const char * ptr = strchr(str, ','); ptr != 0; ptr = strchr(str, ','))
  {
    while ((*str == ' ') || (*str == '\t')) str++;
    const unsigned int length = ptr-str;
    char tmp[length+1];
    strncpy(tmp,  str, length);
    tmp[length] = 0;
    char * tmpptr = tmp;
    while ((*tmpptr != ' ') && (*tmpptr != '\t') && (*tmpptr != 0)) tmpptr++;
    (*tmpptr) = 0;
    locales.push_back(tmp);
    str = ptr+1;
  }
  if ((*str != 0) && (strlen(str) > 0))
  {
    while ((*str == ' ') || (*str == '\t')) str++;
    const unsigned int length = strlen(str);
    char tmp[length+1];
    strncpy(tmp,  str, length);
    tmp[length] = 0;
    char * tmpptr = tmp;
    while ((*tmpptr != ' ') && (*tmpptr != '\t') && (*tmpptr != 0)) tmpptr++;
    (*tmpptr) = 0;
    locales.push_back(tmp);
  }

  init(locales, defaultLocale);
}

typedef std::list<std::string> _stringlist;
void ResourceManager::init(const _stringlist & localeNames, const std::string & defaultLocaleName) throw()
{
  log4cpp::Category &logger(smsc::util::Logger::getCategory("smsc.resourcemanager.ResourceManager"));
  defaultLocale = defaultLocaleName;
  instance.reset(new ResourceManager);
  bool isDefaultLocaleFound = false;
  for (_stringlist::const_iterator i = localeNames.begin(); i != localeNames.end(); i++)
  {
    const std::string & name = *i;
    if (instance->locales.find(name) == instance->locales.end())
    {
      logger.error("Resource file for locale \"%s\" not found.", name.c_str());
      continue;
    }
    isDefaultLocaleFound |= (name == defaultLocaleName);
  }

  if (!isDefaultLocaleFound)
  {
    logger.error("Default locale \"%s\" not found in locales.", defaultLocaleName.c_str());
  }
  instance.get()->validLocales=localeNames;
}

void ResourceManager::reload() throw ()
{
  instance.reset(new ResourceManager);
}

ResourceManager::ResourceManager() throw ()
{
  MutexGuard g(mtx);
  log4cpp::Category &logger(smsc::util::Logger::getCategory("smsc.resourcemanager.ResourceManager"));
  const char * const prefix = "resources_";
  const char * const suffix = ".xml";
  unsigned int prefixLength = strlen(prefix);
  unsigned int suffixLength = strlen(suffix);

  const char * dirName = "";
  DIR* configDir = opendir(dirName = "conf");
  if (configDir == 0)
    configDir = opendir(dirName = "../conf");
  if (configDir == 0)
    configDir = opendir(dirName = ".");
  if (configDir == 0)
  {
    logger.error("Config dir not found");
  }

  for (dirent* entry = readdir(configDir); entry != 0; entry = readdir(configDir))
  {
    unsigned int entryLength = strlen(entry->d_name);
    const char * const entrySuffix = entry->d_name + (entryLength - suffixLength);
    if ((strncmp(entry->d_name,  prefix, prefixLength) == 0)
        && (strcmp(entrySuffix, suffix) == 0))
    {
      std::string name(dirName);
      name += '/';
      name += entry->d_name;
      unsigned int localeNameLength = entryLength - (prefixLength + suffixLength);
      char localeName[entryLength+1];
      strncpy(localeName, entry->d_name + prefixLength, localeNameLength);
      localeName[localeNameLength] = 0;
      locales[localeName] = new LocaleResources(name);
    }
  }
}

ResourceManager::~ResourceManager() throw ()
{
  MutexGuard g(mtx);
  for (_LocalesMap::iterator i = locales.begin(); i != locales.end(); i++)
  {
    delete i->second;
  }
}



// возвращает строку из сетингов для определленой локали и ключа.
std::string ResourceManager::getSetting(const std::string& locale,const std::string& key) const throw ()
{
  MutexGuard g(mtx);
  _LocalesMap::const_iterator l = locales.find(locale);
  if (l != locales.end())
    return (l->second)->getSetting(key);
  else
  {
    l = locales.find(defaultLocale);
    if (l != locales.end())
      return (l->second)->getSetting(key);
    else
      return getSetting(key);
  }
}

// возвращает строку из сетингов для дефолтной локали и ключа.
std::string ResourceManager::getSetting(const std::string& key) const throw ()
{
  MutexGuard g(mtx);
  _LocalesMap::const_iterator l = locales.find(defaultLocale);
  if (l != locales.end())
    return (l->second)->getSetting(key);
  else
    return "";
}

// возвращает строку из ресурса для определленой локали и ключа.
std::string ResourceManager::getString(const std::string& locale, const std::string& key) const throw ()
{
  MutexGuard g(mtx);
  _LocalesMap::const_iterator l = locales.find(locale);
  if (l != locales.end() && l->second->hasString(key))
    return (l->second)->getString(key);
  else
  {
    l = locales.find(defaultLocale);
    if (l != locales.end())
      return (l->second)->getString(key);
    else
    {
      __trace2__("RM: string not found for locale=%s, key=%s",locale.c_str(),key.c_str());
      return "";
    }
  }
}

// возвращает строку из ресурса для дефолтной локали и ключа.
std::string ResourceManager::getString(const std::string& key) const throw ()
{
  MutexGuard g(mtx);
  _LocalesMap::const_iterator l = locales.find(defaultLocale);
  if (l != locales.end())
    return (l->second)->getString(key);
  else
  {
    __trace2__("RM: string not found for locale=%s, key=%s",defaultLocale.c_str(),key.c_str());
    return "";
  }
}

OutputFormatter* ResourceManager::getFormatter(const std::string& locale, const std::string& key) const throw()
{
  MutexGuard g(mtx);
  _LocalesMap::const_iterator l = locales.find(locale);
  if (l != locales.end() && l->second->hasString(key))
    return (l->second)->getFormatter(key);
  else
  {
    l = locales.find(defaultLocale);
    if (l != locales.end())
      return (l->second)->getFormatter(key);
    else
    {
      __trace2__("RM: formatter not found for locale=%s, key=%s",locale.c_str(),key.c_str());
      return 0;
    }
  }
}

OutputFormatter* ResourceManager::getFormatter(const std::string& key) const throw()
{
  MutexGuard g(mtx);
  const _LocalesMap::const_iterator l = locales.find(defaultLocale);
  if (l != locales.end())
    return (l->second)->getFormatter(key);
  else
  {
    __trace2__("RM: formatter not found for locale=%s, key=%s",defaultLocale.c_str(),key.c_str());
    return 0;
  }

}


bool ResourceManager::isValidLocale(const std::string& localeName)const
{
  MutexGuard g(mtx);
  for(_stringlist::const_iterator i=validLocales.begin();i!=validLocales.end();i++)
  {
    if(localeName==*i)return true;
  }
  return false;
}


#ifdef SMSC_DEBUG
void ResourceManager::dump(std::ostream & outStream) const
{
  outStream << "ResourceManager dump: " << std::endl;
  for (_LocalesMap::const_iterator i = locales.begin(); i != locales.end(); i++)
  {
    outStream << "Locale: " << i->first << std::endl;
    i->second->dump(outStream);
  }
}
#endif //#ifdef SMSC_DEBUG

};//resourcemanager
};//smsc
