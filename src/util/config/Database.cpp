#include "Database.h"
#include "XmlUtils.h"
#include <xercesc/dom/DOM_NodeList.hpp>
#include <util/Logger.h>

namespace smsc   {
namespace util   {
namespace config {

const DOMString instance_name(createDOMString("instance"));
const DOMString user_name(createDOMString("userName"));
const DOMString password_name(createDOMString("userPassword"));
const DOMString connections_name(createDOMString("connections"));
const DOMString connections_max_name(createDOMString("max"));
const DOMString connections_init_name(createDOMString("init"));

Database::Database(DOM_Element & config_node)
	: logger(smsc::util::Logger::getCategory("smsc.util.config.Database"))
{
	node = config_node;
	instance = node.getAttribute(instance_name).transcode();
	user = node.getAttribute(user_name).transcode();
	password = node.getAttribute(password_name).transcode();
	DOM_NodeList list = node.getElementsByTagName(connections_name);
	if (list.getLength() > 0)
	{
		DOM_Node n = list.item(0);
		DOM_Element *e = (DOM_Element *)&n;
		char * str = e->getAttribute(connections_max_name).transcode();
		max = atoi(str);
		delete[] str;

		str = e->getAttribute(connections_init_name).transcode();
		init = atoi(str);
		delete[] str;
	}
}

void Database::setDBInstance(const char * const newInstance)
	throw (DOM_DOMException)
{
	node.setAttribute(instance_name, newInstance);
	replaceString(instance, newInstance);
}

Database::~Database()
{
}

void Database::setDBUserName(const char * const newUserName)
	throw (DOM_DOMException)
{
	node.setAttribute(user_name, newUserName);
	replaceString(user, newUserName);
}

void Database::setDBUserPassword(const char * const newPassword)
	throw (DOM_DOMException)
{
	node.setAttribute(password_name, newPassword);
	replaceString(password, newPassword);
}

void Database::setMaxConnectionsCount(int newMax)
	throw (DOM_DOMException)
{
	char str[32];
	snprintf(str, sizeof(str), "%.8i", newMax);
	node.setAttribute(connections_max_name, str);
}

void Database::setInitConnectionsCount(int newInit)
	throw (DOM_DOMException)
{
	char str[32];
	snprintf(str, sizeof(str), "%.8i", newInit);
	node.setAttribute(connections_init_name, str);
}

}
}
}

