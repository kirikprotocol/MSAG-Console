#ifndef CONFIGURATIONMANAGER_H_INCLUDED_C3A81BD4
#define CONFIGURATIONMANAGER_H_INCLUDED_C3A81BD4

#include <iostream>

#include <xercesc/dom/DOM_Document.hpp>
#include <log4cpp/Category.hh>

#include "Database.h"
#include "MapProtocol.h"
#include "Log.h"
#include "ConfigException.h"

#include <store/StoreConfig.h>

namespace smsc   {
namespace util   {
namespace config {

using smsc::store::StoreConfig;

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
		manager = 0;
		config_filename = new char[strlen(configurationFileName)+1];
		strcpy(config_filename, configurationFileName);
	}

	/**
	 * Возвращает проинициализированный Manager
	 */
	static Manager & getInstance()
	{
		if (manager == 0)
		{
			manager = new Manager();
		}
		return *manager;
	}

	/**
	 * Запись конфигурации
	 */
	void save();
	/**
	 * Возвращает конфигурацию базы данных.
	 *
	 * @return Конфигурация базы данных
	 */
	StoreConfig *getStoreConfig() const {return db;};
	/**
	 * Возвращает конфигурацию логгера
	 *
	 * @return logger configuration
	 */
	Log *getLog() const {return log;};
	/**
	 * Возвращает настройки протокола MAP
	 *
	 * @return MAP protocol configuration
	 */
	MapProtocol *getMapProtocol() const {return map;};

protected:
	/**
	 * Читает конфигурацию.
	 *
	 * @param config_filename
	 *               Имя файла, в котором хранится конфигурация.
	 */
	Manager() throw(ConfigException &);
	static Manager * manager;

private:
	static char * config_filename;
	void writeNode(std::ostream &out, DOM_Node & node, unsigned int tabs);
	void writeHeader(std::ostream &out);
	DOM_Document document;
	Database *db;
	MapProtocol *map;
	Log *log;
	log4cpp::Category &cat;

	DOMParser * createParser();
	void parse(DOMParser *parser, const char * const filename) throw (ConfigException &);
	void processTree(const DOM_Element &element);

};

}
}
}

#endif /* CONFIGURATIONMANAGER_H_INCLUDED_C3A81BD4 */
