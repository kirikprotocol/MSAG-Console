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
	 *
	 * @param paramName имя параметра
	 * @return значение параметра типа int
	 * @see getString()
	 * @see getBool()
	 */
	long getInt(const char * const paramName)
	{
		if (longParams.Exists(paramName))
		{
			return longParams[paramName];
		}
		else
		{
			std::string msg;
			msg += "Integer parameter ";
			msg += paramName;
			msg += "not found";
			throw ConfigException(msg.c_str());
		}
	}

	/**
	 *
	 * @param paramName имя параметра
	 * @return значение параметра типа String
	 */
	char * getString(const char * const paramName)
		throw (ConfigException &)
	{
		if (strParams.Exists(paramName))
			return strParams[paramName];
		else
		{
			std::string msg;
			msg += "String parameter ";
			msg += paramName;
			msg += "not found";
			throw ConfigException(msg.c_str());
		}
	}

	/**
	 *
	 * @param paramName имя параметра
	 * @return значение параметра типа Bool
	 * @see getInt()
	 * @see getString()
	 */
	bool getBool(const char * const paramName)
	{
		if (boolParams.Exists(paramName))
		{
			return boolParams[paramName];
		}
		else
		{
			std::string msg;
			msg += "Boolean parameter ";
			msg += paramName;
			msg += "not found";
			throw ConfigException(msg.c_str());
		}
	}

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
	Hash<long> longParams;
	Hash<char *> strParams;
	Hash<bool> boolParams;
	log4cpp::Category &logger;

	DOMParser * createParser();
	DOM_Document parse(DOMParser *parser, const char * const filename) throw (ConfigException &);
	void processTree(const DOM_Element &element) throw (ConfigException &);
	void Manager::processNode(const DOM_Element &element, const char * const prefix) throw (DOM_DOMException &);
	void Manager::processParamNode(const DOM_Element &element, const char * const name, const char * const type) throw (DOM_DOMException &);
};

}
}
}

#endif /* CONFIGURATIONMANAGER_H_INCLUDED_C3A81BD4 */
