#ifndef SMSC_ADMIN_ADMIN_EXCEPTION
#define SMSC_ADMIN_ADMIN_EXCEPTION

#include <util/Exception.hpp>

namespace smsc {
namespace admin {

using smsc::util::Exception;

class AdminException : public Exception
{
public:
	AdminException(const AdminException &a);
	AdminException(const char * const exceptionMessage, ...)
		: Exception()
	{SMSC_UTIL_EX_FILL(exceptionMessage);}
};

}
}
#endif // ifndef SMSC_ADMIN_ADMIN_EXCEPTION
