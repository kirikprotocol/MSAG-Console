#ifndef DBCONFIGURATION_H_INCLUDED_C3A87A6B
#define DBCONFIGURATION_H_INCLUDED_C3A87A6B

#include <xercesc/dom/DOM_Element.hpp>
#include <xercesc/dom/DOM_DOMException.hpp>
#include <log4cpp/Category.hh>
#include <util/config/ConfigException.h>

namespace smsc   {
namespace util   {
namespace config {

/**
 * Настройки базы данных
 *
 * @author igork
 * @see Manager
 */
class Database
{
public:
	Database(DOM_Element & config_node);
	const char * const getInstance() const {return instance;};
	const char * const getUserName() const {return user;};
	const char * const getPassword() const {return password;};
	const int getConnectionsMax() const {return max;};
	const int getConnectionsInit() const {return init;};
	void setInstance(const char * const newInstance) throw (DOM_DOMException);
	void setUserName(const char * const newUserName) throw (DOM_DOMException);
	void setPassword(const char * const newPassword) throw (DOM_DOMException);
	void setConnectionsMax(int newMax) throw (DOM_DOMException);
	void setConnectionsInit(int newInit) throw (DOM_DOMException);

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
