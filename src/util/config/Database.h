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
	const char * const getUrl() const {return url;};
	const char * const getUserName() const {return user;};
	const char * const getPassword() const {return password;};
	void setUrl(const char * const newUrl) throw (DOM_DOMException);
	void setUserName(const char * const newUserName) throw (DOM_DOMException);
	void setPassword(const char * const newPassword) throw (DOM_DOMException);

private:
	char *url;
	char* user;
	char* password;
	DOM_Element node;

	log4cpp::Category &logger;

	static const DOMString url_name;
	static const DOMString user_name;
	static const DOMString password_name;
};

}
}
}

#endif /* DBCONFIGURATION_H_INCLUDED_C3A87A6B */
