#ifndef SMSC_ADMIN_SERVICE_SERVICE_SOCKET_LISTENER
#define SMSC_ADMIN_SERVICE_SERVICE_SOCKET_LISTENER

#include <admin/AdminException.h>

namespace smsc {
namespace admin {
namespace service {

using smsc::admin::AdminException;

class ServiceSocketListener {
public:
	ServiceSocketListener(unsigned int port) throw(AdminException&);
	void Run();

protected:

private:
	unsigned int port;
	int sock;
};

}
}
}

#endif // ifndef SMSC_ADMIN_SERVICE_SERVICE_SOCKET_LISTENER
