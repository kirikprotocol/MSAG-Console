#ifndef DBCONFIGURATION_H_INCLUDED_C3A87A6B
#define DBCONFIGURATION_H_INCLUDED_C3A87A6B

#include <xercesc/dom/DOM_Node.hpp>

namespace smsc   {
namespace util   {
namespace config {

class DB
{
public:
	const char *getUrl() const;
	const char *getUserName() const;
	const char *getPassword() const;
	void setUrl(const char * const new_url);
	void setUserName(const char * const new_user_name);
	void setPassword(const char * const new_password);

private:
	char *url;
	char* user;
	char* password;
	DOM_Node node;
};

}
}
}

#endif /* DBCONFIGURATION_H_INCLUDED_C3A87A6B */
