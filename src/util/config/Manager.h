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
	 * @see getInt()
	 * @see getBool()
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

private:
	class ConfigParam
	{
	public:
		char * name;
		enum types {intType, stringType, boolType} type;
		char * value;

		ConfigParam(const char * const _name,
								types _type,
								const char * const _value)
		{
			name = strdup(_name);
			type = _type;
			value = strdup(_value);
		}
		ConfigParam(const ConfigParam &copy)
		{
			name = strdup(copy.name);
			type = copy.type;
			value = strdup(copy.value);
		}
		~ConfigParam()
		{
			free(name);
			free(value);
		}
	};
	class ConfigTree
	{
	public:
		ConfigTree(const char * const _name)
		{
			name = strdup(_name);
		}
		/*ConfigTree(const ConfigTree &copy)
		{
			name = strdup(copy.name);
			params = copy.params;
			sections = copy.sections;
		}*/
		~ConfigTree()
		{
			ConfigTree * value;
			char * _name;
			for (sections.First(); sections.Next(_name, value);)
			{
				delete value;
				sections[_name]=0;
			}
			params.clear();
			free(_name);
		}

		void addParam(const char * const name,
		              ConfigParam::types type,
		              const char * const value);
		void write(std::ostream &out, std::string prefix);
	private:
		Hash<ConfigTree*>	sections;
		std::vector<ConfigParam> params;
		char * name;
		ConfigTree* createSection(const char * const name);
	};
	static char * config_filename;
	ConfigTree * createTree();
	void writeHeader(std::ostream &out);
	void writeFooter(std::ostream &out);
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
