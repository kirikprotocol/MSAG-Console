#include "Database.h"
#include "XmlUtils.h"
#include <util/Logger.h>

namespace smsc   {
namespace util   {
namespace config {

const DOMString Database::url_name(createDOMString("url"));
const DOMString Database::user_name(createDOMString("user"));
const DOMString Database::password_name(createDOMString("password"));

Database::Database(DOM_Element & config_node)
	: logger(smsc::util::Logger::getCategory("smsc.util.config.Database"))
{
	node = config_node;
	url = node.getAttribute(url_name).transcode();
	user = node.getAttribute(user_name).transcode();
	password = node.getAttribute(password_name).transcode();
}

void Database::setUrl(const char * const newUrl)
	throw (DOM_DOMException)
{
	node.setAttribute(url_name, newUrl);
	replaceString(url, newUrl);
}

void Database::setUserName(const char * const newUserName)
	throw (DOM_DOMException)
{
	node.setAttribute(user_name, newUserName);
	replaceString(user, newUserName);
}

void Database::setPassword(const char * const newPassword)
	throw (DOM_DOMException)
{
	node.setAttribute(password_name, newPassword);
	replaceString(password, newPassword);
}

}
}
}

