#ifndef CONFIGURATIONMANAGER_H_INCLUDED_C3A81BD4
#define CONFIGURATIONMANAGER_H_INCLUDED_C3A81BD4

#include <iostream>

#include <xercesc/dom/DOM_Document.hpp>
#include <log4cpp/Category.hh>

#include "Database.h"
#include "MapProtocol.h"
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
	Database &getDatabase() const {return *db;};
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
	MapProtocol &getMapProtocol() const {return *map;};

private:
	void writeNode(std::ostream &out, DOM_Node & node, unsigned int tabs);
	void writeHeader(std::ostream &out);
	const char * config_filename;
	DOM_Document document;
	Database *db;
	MapProtocol *map;
	Log *log;
	log4cpp::Category &cat;

	DOMParser * createParser();
	void parse(DOMParser *parser, const char * const filename) throw (ConfigException &);
	void processTree(const DOM_Element &element);

	static const DOMString db_name;
	static const DOMString map_name;
	static const DOMString log_name;
};

}
}
}

#endif /* CONFIGURATIONMANAGER_H_INCLUDED_C3A81BD4 */
