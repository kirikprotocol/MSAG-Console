#ifndef SMSC_ADMIN_UTIL_SHUTDOWNABLE
#define SMSC_ADMIN_UTIL_SHUTDOWNABLE

#include <signal.h>

namespace smsc {
namespace admin {
namespace util {

class Shutdownable
{
public:
	virtual void shutdown() = 0;
};

}
}
}
#endif // ifndef SMSC_ADMIN_UTIL_SHUTDOWNABLE
