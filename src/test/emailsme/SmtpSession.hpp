#ifndef TEST_EMAIL_SMTP_SESSION
#define TEST_EMAIL_SMTP_SESSION

#include "core/network/Socket.hpp"
#include "util/Exception.hpp"

using smsc::core::network::Socket;
using smsc::util::Exception;

namespace smsc {
namespace test {
namespace emailsme {

class SmtpSession
{
	bool connected;
	Socket s;

	void checkOutput(const char* outputStart, const char* outputEnd);
	void smtpCmd(const char* input, const char* outputStart, const char* outputEnd);

public:
	SmtpSession() : connected(false) {}
	~SmtpSession()
	{
		if (connected) { quitCmd(); }
	}
	void connect(const char* smtpHost, int smtpPort = 25);
	void mailCmd(const char* reversePath);
	void rcptCmd(const char* forwardPath);
	void dataCmd(const char* msg);
	void quitCmd();
};

}
}
}

#endif /* TEST_EMAIL_SMTP_SESSION */
