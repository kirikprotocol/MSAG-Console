#include "AdminException.h"

#include <string.h>

namespace smsc {
namespace admin {

AdminException::AdminException(const char *	const message)
	throw ()
	: std::exception()
{
	msg = new char[strlen(message)+1];
	strcpy(msg, message);
}

AdminException::AdminException(const AdminException &a)
	throw ()
	: std::exception()
{
	msg = new char[strlen(a.msg)+1];
	strcpy(msg, a.msg);
}

/*AdminException::AdminException(AdminException a) {
	msg = new char[strlen(a.msg)+1];
	strcpy(msg, a.msg);
}*/

AdminException::~AdminException()
	throw ()
{
	delete[] msg;
}

}
}

