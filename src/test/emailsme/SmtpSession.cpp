#include "SmtpSession.hpp"
#include "util/debug.h"
#include <string>
#include <sstream>

namespace smsc {
namespace test {
namespace emailsme {

using namespace std;

void SmtpSession::checkOutput(const char* outputStart, const char* outputEnd)
{
	if (!outputStart && !outputEnd)
	{
		return;
	}
	char buf[256];
	if (s.Gets(buf, sizeof(buf)) <= 0)
	{
		throw Exception("socket gets() failed");
	}
	__trace2__("%s", buf);
	string output(buf);
	if (outputStart && output.find_first_of(outputStart) == string::npos)
	{
		throw Exception("outputStart doesn't match");
	}
	if (outputEnd && output.find_last_of(outputEnd) == string::npos)
	{
		throw Exception("outputEnd doesn't match");
	}
}

void SmtpSession::smtpCmd(const char* input, const char* outputStart,
	const char* outputEnd)
{
	__require__(input);
	if (!connected)
	{
		throw Exception("not connected");
	}
	__trace2__("%s", input);
	if (s.Puts(input) <= 0)
	{
		throw Exception("socket put() failed");
	}
	checkOutput(outputStart, outputEnd);
}

void SmtpSession::connect(const char* smtpHost, int smtpPort)
{
	if (s.Init(smtpHost, smtpPort, 8))
	{
		throw Exception("Failed to init socket");
	}
	if (s.Connect())
	{
		throw Exception("Failed to connect to %s:%d", smtpHost, smtpPort);
	}
	checkOutput("220 ", NULL);
	connected = true;
	helo();
}

void SmtpSession::send(const MimeMessage& msg)
{
	mail(msg.getFrom());
	const vector<const char*>& recipients = msg.getRecipients();
	for (int i = 0; i < recipients.size(); i++)
	{
		rcpt(recipients[i]);
	}
	ostringstream os;
	msg.writeTo(os);
	data(os.str().c_str());
}

void SmtpSession::helo()
{
	smtpCmd("helo smsc\n", "250 ", "pleased to meet you");
}

void SmtpSession::mail(const char* reversePath)
{
	__require__(reversePath);
	ostringstream os;
	os << "mail from:" << reversePath << endl;
	smtpCmd(os.str().c_str(), "250 ", "Sender ok");
}

void SmtpSession::rcpt(const char* forwardPath)
{
	__require__(forwardPath);
	ostringstream os;
	os << "rcpt to:" << forwardPath << endl;
	smtpCmd(os.str().c_str(), "250 ", "Recipient ok");
}

void SmtpSession::data(const char* msg)
{
	__require__(msg);
	smtpCmd("data\n", NULL, NULL);
	ostringstream os;
	os << msg << endl << "." << endl;
	smtpCmd(os.str().c_str(), "250 ", "Message accepted for delivery");
}

void SmtpSession::quit()
{
	smtpCmd("quit", "221 ", "closing connection");
	connected = false;
}

}
}
}
