#ifndef CONFIGURATIONMANAGER_H_INCLUDED_C3A81BD4
#define CONFIGURATIONMANAGER_H_INCLUDED_C3A81BD4

#include <iostream>

#include <xercesc/dom/DOM_Document.hpp>
#include <xercesc/dom/DOM_Element.hpp>
#include <xercesc/dom/DOM_DOMException.hpp>
#include <log4cpp/Category.hh>

#include <store/StoreConfig.h>
#include <core/buffers/Hash.hpp>
#include <util/config/ConfigException.h>
#include <util/config/Config.h>

namespace smsc   {
namespace util   {
namespace config {

using smsc::store::StoreConfig;
using smsc::core::buffers::Hash;

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
	{
		config_filename = new char[strlen(configurationFileName)+1];
		strcpy(config_filename, configurationFileName);
		manager = new Manager();
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
		throw (HashInvalidKeyException &)
	{
		return config.getInt(paramName);
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
		throw (HashInvalidKeyException &)
	{
		return config.getString(paramName);
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
	bool getBool(const char * const paramName)
		throw (HashInvalidKeyException &)
	{
		return config.getBool(paramName);
	}

	/**
	 * Запись конфигурации
	 */
	void save();

protected:
	/**
	 * Читает конфигурацию.
	 *
	 * @param config_filename
	 *               Имя файла, в котором хранится конфигурация.
	 */
	Manager() throw(ConfigException &);
	static Manager * manager;
	Config config;

private:
	static char * config_filename;
	void writeHeader(std::ostream &out);
	void writeFooter(std::ostream &out);
	log4cpp::Category &logger;

	DOMParser * createParser();
	DOM_Document parse(DOMParser *parser, const char * const filename) throw (ConfigException &);
};

}
}
}

#endif /* CONFIGURATIONMANAGER_H_INCLUDED_C3A81BD4 */
