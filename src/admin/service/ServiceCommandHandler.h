#ifndef	SERVICE_COMMAND_HANDLER
#define SERVICE_COMMAND_HANDLER

#include <iostream>

namespace smsc {
namespace admin {
namespace service {

class ServiceCommandHandler {
public:
	ServiceCommandHandler(int admSocket);
	void Run();

protected:
private:
	int sock;
};

}
}
}
#endif // ifndef SERVICE_COMMAND_HANDLER

