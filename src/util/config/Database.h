#ifndef DBCONFIGURATION_H_INCLUDED_C3A87A6B
#define DBCONFIGURATION_H_INCLUDED_C3A87A6B

#include <xercesc/dom/DOM_Element.hpp>
#include <xercesc/dom/DOM_DOMException.hpp>
#include <log4cpp/Category.hh>
#include <util/config/ConfigException.h>
#include <store/StoreConfig.h>

namespace smsc   {
namespace util   {
namespace config {

using smsc::store::StoreConfig;

/**
 * Настройки базы данных
 *
 * @author igork
 * @see Manager
 */
class Database : public StoreConfig
{
public:
	Database(DOM_Element & config_node);
	Database::~Database();
	virtual const char* getDBInstance()     {return instance;}
	virtual const char* getDBUserName()     {return user;}
	virtual const char* getDBUserPassword() {return password;}

	virtual int getMaxConnectionsCount()  {return max;};
	virtual int getInitConnectionsCount() {return init;};
	
	void setDBInstance(const char * const newInstance) throw (DOM_DOMException);
	void setDBUserName(const char * const newUserName) throw (DOM_DOMException);
	void setDBUserPassword(const char * const newPassword) throw (DOM_DOMException);
	void setMaxConnectionsCount(int newMax) throw (DOM_DOMException);
	void setInitConnectionsCount(int newInit) throw (DOM_DOMException);

private:
	char *instance;
	char *user;
	char *password;
	int max;
	int init;
	DOM_Element node;

	log4cpp::Category &logger;
};

}
}
}

#endif /* DBCONFIGURATION_H_INCLUDED_C3A87A6B */
