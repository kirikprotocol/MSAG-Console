#ifndef CONFIGURATIONMANAGER_H_INCLUDED_C3A81BD4
#define CONFIGURATIONMANAGER_H_INCLUDED_C3A81BD4

#include <xercesc/dom/DOM_Document.hpp>
#include <log4cpp/Category.hh>

#include "Db.h"
#include "Map.h"
#include "Log.h"
#include "ConfigException.h"

namespace smsc   {
namespace util   {
namespace config {

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
	/**
	 * ������ ������������.
	 *
	 * @param config_filename
	 *               ��� �����, � ������� �������� ������������.
	 */
	Manager(const char * const config_filename) throw(ConfigException &);
	/**
	 * ������ ������������
	 */
	void save();
	/**
	 * ���������� ������������ ���� ������.
	 *
	 * @return ������������ ���� ������
	 */
	Db &getDb() const {return *db;};
	/**
	 * ���������� ������������ �������
	 *
	 * @return logger configuration
	 */
	Log &getLog() const {return *log;};
	/**
	 * ���������� ��������� ��������� MAP
	 *
	 * @return MAP protocol configuration
	 */
	Map &getMap() const {return *map;};

private:
	const char * config_filename;
	DOM_Document document;
	Db *db;
	Map *map;
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
