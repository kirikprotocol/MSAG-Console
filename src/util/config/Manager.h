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
 * �����, ���������� �� ������ � ������ ������������ �������.
 * ������ ������������ � ������������, ������ - ������� save()
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
		throw (ConfigException &)
	{
		config_filename = new char[strlen(configurationFileName)+1];
		strcpy(config_filename, configurationFileName);
		manager = new Manager();
	}

	/**
	 * ���������� ��������������������� Manager
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
	 * @param paramName ��� ���������
	 * @return �������� ��������� ���� int
	 * @exception HashInvalidKeyException
	 *                   if parameter not found
	 * @see getString()
	 * @see getBool()
	 */
	int32_t getInt(const char * const paramName)
		throw (ConfigException &)
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
	 * @param paramName ��� ���������
	 * @return �������� ��������� ���� String
	 * @exception HashInvalidKeyException
	 *                   if parameter not found
	 * @see getInt()
	 * @see getBool()
	 */
	char * getString(const char * const paramName)
		throw (ConfigException &)
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
	 * @param paramName ��� ���������
	 * @return �������� ��������� ���� Bool
	 * @exception HashInvalidKeyException
	 *                   if parameter not found
	 * @see getInt()
	 * @see getString()
	 */
	bool getBool(const char * const paramName)
		throw (ConfigException &)
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

	/**
	 * ������ ������������
	 */
	void save();

protected:
	/**
	 * ������ ������������.
	 *
	 * @param config_filename
	 *               ��� �����, � ������� �������� ������������.
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
