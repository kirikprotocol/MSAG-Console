#ifndef DBCONFIGURATION_H_INCLUDED_C3A87A6B
#define DBCONFIGURATION_H_INCLUDED_C3A87A6B

#include <xercesc/dom/DOM_Element.hpp>

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
	const char *getUrl() const {return url;};
	const char *getUserName() const {return user;};
	const char *getPassword() const {return password;};
	void setUrl(const char * const new_url);
	void setUserName(const char * const new_user_name);
	void setPassword(const char * const new_password);

private:
	char *url;
	char* user;
	char* password;
	DOM_Element& node;

	static const DOMString url_name;
	static const DOMString user_name;
	static const DOMString password_name;
};

}
}
}

#endif /* DBCONFIGURATION_H_INCLUDED_C3A87A6B */
