#include <resourcemanager/ResourceManager.hpp>
#include <string>
#include <iostream>
#include <dirent.h>
#include <util/Exception.hpp>

namespace smsc{
namespace resourcemanager{

using smsc::util::Exception;

std::auto_ptr<ResourceManager> ResourceManager::instance(0);
const std::string ResourceManager::defaultLocale("en_en");

ResourceManager& ResourceManager::getInstance()
{
	if (instance.get() == 0)
	{
		instance.reset(new ResourceManager);
	}
	return *instance;
}

ResourceManager::ResourceManager()
	throw (Exception)
{
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
		throw Exception("Config dir not found");

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

ResourceManager::~ResourceManager()
{
	for (_LocalesMap::iterator i = locales.begin(); i != locales.end(); i++)
	{
		delete i->second;
	}
}



// возвращает строку из сетингов для определленой локали и ключа.
std::string ResourceManager::getSetting(const std::string& locale,const std::string& key)
{
	if (locales.find(locale) != locales.end())
		return locales[locale]->getSetting(key);
	else
		return "";
}

// возвращает строку из сетингов для дефолтной локали и ключа.
std::string ResourceManager::getSetting(const std::string& key )
{
	if (locales.find(defaultLocale) != locales.end())
		return locales[defaultLocale]->getSetting(key);
	else
		return "";
}

// возвращает строку из ресурса для определленой локали и ключа.
std::string ResourceManager::getString(const std::string& locale, const std::string& key)
{
	if (locales.find(locale) != locales.end())
		return locales[locale]->getString(key);
	else
		return "";
}

// возвращает строку из ресурса для дефолтной локали и ключа.
std::string ResourceManager::getString(const std::string& key)
{
	if (locales.find(defaultLocale) != locales.end())
		return locales[defaultLocale]->getString(key);
	else
		return "";
}

#ifdef SMSC_DEBUG
void ResourceManager::dump(std::ostream & outStream)
{
	outStream << "ResourceManager dump: " << std::endl;
	for (_LocalesMap::iterator i = locales.begin(); i != locales.end(); i++)
	{
		outStream << "Locale: " << i->first << std::endl;
		i->second->dump(outStream);
	}
}
#endif //#ifdef SMSC_DEBUG

};//resourcemanager
};//smsc

