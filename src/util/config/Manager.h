#ifndef CONFIGURATIONMANAGER_H_INCLUDED_C3A81BD4
#define CONFIGURATIONMANAGER_H_INCLUDED_C3A81BD4

#include <iostream>

#include <xercesc/dom/DOM_Document.hpp>
#include <xercesc/dom/DOM_Element.hpp>
#include <xercesc/dom/DOM_DOMException.hpp>
#include <log4cpp/Category.hh>

#include <store/StoreConfig.h>
#include <core/buffers/Hash.hpp>
#include <util/cstrings.h>
#include <util/config/ConfigException.h>
#include <util/config/Config.h>

namespace smsc   {
namespace util   {
namespace config {

using smsc::store::StoreConfig;
using smsc::core::buffers::Hash;
using smsc::util::cStringCopy;

/**
 * Класс, отвечающий за чтение и запись конфигурации системы.
 * Чтение производится в конструкторе, запись - методом save()
 *
 * @author igork
 * @see Db
 * @see Map
 * @see Log
 * @see save()
 */
class Manager
{
public:
	static void init(const char * const configurationFileName)
		throw (ConfigException)
	{
		if (manager != 0)
			throw ConfigException("Configuration manager already initialized");
		if (config_filename != 0)
			delete config_filename;
		config_filename = cStringCopy(configurationFileName);
		manager = new Manager();
	}

	static void deinit()
	{
		if (manager != 0)
		{
			delete manager;
			manager = 0;
		}
		if (config_filename != 0)
		{
			delete config_filename;
			config_filename = 0;
		}
	}

	/**
	 * Возвращает проинициализированный Manager
	 */
	static Manager & getInstance()
	{
		return *manager;
	}

	smsc::util::config::Config & getConfig()
	{
		return config;
	}

	/**
	 *
	 * @param paramName имя параметра
	 * @return значение параметра типа int
	 * @exception HashInvalidKeyException
	 *                   if parameter not found
	 * @see getString()
	 * @see getBool()
	 */
	int32_t getInt(const char * const paramName)
		throw (ConfigException)
	{
		try {
			return config.getInt(paramName);
		}
		catch (HashInvalidKeyException &e)
		{
			std::string s("Int key \"");
			s += paramName;
			s += "\" not found";
			throw ConfigException(s.c_str());
		}
	}

	/**
	 *
	 * @param paramName имя параметра
	 * @return значение параметра типа String
	 * @exception HashInvalidKeyException
	 *                   if parameter not found
	 * @see getInt()
	 * @see getBool()
	 */
	char * getString(const char * const paramName)
		throw (ConfigException)
	{
		try {
			return config.getString(paramName);
		}
		catch (HashInvalidKeyException &e)
		{
			std::string s("String key \"");
			s += paramName;
			s += "\" not found";
			throw ConfigException(s.c_str());
		}
	}

	/**
	 *
	 * @param paramName имя параметра
	 * @return значение параметра типа Bool
	 * @exception HashInvalidKeyException
	 *                   if parameter not found
	 * @see getInt()
	 * @see getString()
	 */
	bool getBool(const char * const paramName) const
		throw (ConfigException)
	{
		try {
			return config.getBool(paramName);
		}
		catch (HashInvalidKeyException &e)
		{
			std::string s("Bool key \"");
			s += paramName;
			s += "\" not found";
			throw ConfigException(s.c_str());
		}
	}

	void setInt(const char * const paramName, int32_t value)
	{
		config.setInt(paramName, value);
	}

	void setString(const char * const paramName, const char * const value)
	{
		config.setString(paramName, value);
	}
	
	void setBool(const char * const paramName, bool value)
	{
		config.setBool(paramName, value);
	}

	/**
	 * Запись конфигурации
	 */
	void save();

	void removeSection(const char * const sectionName)
	{
		config.removeSection(sectionName);
	}

	CStrSet *getChildSectionNames(const char * const sectionName)
	{
		return config.getChildSectionNames(sectionName);
	}

protected:
	/**
	 * Читает конфигурацию.
	 *
	 * @param config_filename
	 *               Имя файла, в котором хранится конфигурация.
	 */
	Manager() throw(ConfigException);
	static Manager * manager;
	Config config;

private:
	static char * config_filename;
	void writeHeader(std::ostream &out);
	void writeFooter(std::ostream &out);

	DOMParser * createParser();
	DOM_Document parse(DOMParser *parser, const char * const filename) throw (ConfigException);
};

}
}
}

#endif /* CONFIGURATIONMANAGER_H_INCLUDED_C3A81BD4 */
