#include "Database.h"
#include "XmlUtils.h"

namespace smsc   {
namespace util   {
namespace config {

const DOMString Database::url_name(createDOMString("url"));
const DOMString Database::user_name(createDOMString("user"));
const DOMString Database::password_name(createDOMString("password"));

Database::Database(DOM_Element & config_node) : node(config_node) {
	url = config_node.getAttribute(url_name).transcode();
	user = config_node.getAttribute(user_name).transcode();
	password = config_node.getAttribute(password_name).transcode();
}

void Database::setUrl(const char * const new_url)
{
	if (new_url != 0)
	{
		node.setAttribute(url_name, new_url);
	}
}

void Database::setUserName(const char * const new_user_name)
{
	if (new_user_name != 0)
	{
		node.setAttribute(user_name, new_user_name);
	}
}

void Database::setPassword(const char * const new_password)
{
	if (new_password != 0)
	{
		node.setAttribute(password_name, new_password);
	}
}

}
}
}

