#include "AdminException.h"

#include <string.h>

namespace smsc {
namespace admin {

AdminException::AdminException(const char *	const message)
{
	msg = new char[strlen(message)+1];
	strcpy(msg, message);
}

AdminException::~AdminException()
{
	delete[] msg;
}

}
}

