#include "AdminException.h"

namespace smsc {
namespace admin {

AdminException::AdminException(const char *	const msg)
	: Exception(msg)
{}

AdminException::AdminException(const AdminException &a)
	: Exception(a.what())
{}

}
}

