#include "ServiceCommandHandler.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <fstream>

namespace smsc {
namespace admin {
namespace service {

ServiceCommandHandler::ServiceCommandHandler(int admSocket)
{
	sock = admSocket;
}

void ServiceCommandHandler::Run() {
	int n = 0;
	char buf[20];

	send(sock, "Hello\n\r", sizeof("Hello\n\r"), 0);
	
	while ((n = recv(sock, buf, sizeof(buf), 0)) != 0)
	{
		for (unsigned int i=0; i<n; i++)
		{
			buf[i]++;
		}
		send(sock, buf, n, 0);
	}
	close(sock);
}

}
}
}

