#ifndef TEST_EMAIL_SMTP_SESSION
#define TEST_EMAIL_SMTP_SESSION

#include "MimeMessage.hpp"
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
		if (connected) { quit(); }
	}
	void connect(const char* smtpHost, int smtpPort = 25);
	void send(const MimeMessage& msg);
	void helo();
	void mail(const char* reversePath);
	void rcpt(const char* forwardPath);
	void data(const char* msg);
	void quit();
};

}
}
}

#endif /* TEST_EMAIL_SMTP_SESSION */

