#include "MimeMessage.hpp"
#include "test/util/Util.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace emailsme {

using namespace std;
using namespace smsc::test::util;

Multipart::Multipart() : number(rand1(INT_MAX)) {}
Multipart::~Multipart()
{
	for (int i = 0; i < parts.size(); i++)
	{
		delete parts[i];
	}
}

void Multipart::writeTo(ostream& os) const
{
	for (int i = 0; i < parts.size(); i++)
	{
		os << "------=_NextPart_" << number << endl;
		os << parts[i];
	}
	os << "------=_NextPart_" << number << "--";
}

Part::~Part()
{
	if (mp) { delete mp; }
}

void Part::setHeader(const char* name, const char* value)
{
	__require__(name);
	__require__(value);
	headers[name] = value;
}

void Part::setContent(const char* _content)
{
	__require__(_content);
	content = _content;
	if (mp) { delete mp; mp = NULL; }
}

void Part::setContent(const Multipart* _mp)
{
	__require__(_mp);
	if (mp) { delete mp; }
	mp = _mp;
	content = "";
}

void Part::writeTo(ostream& os) const
{
	for (Headers::const_iterator it = headers.begin(); it != headers.end(); it++)
	{
		if (mp && it->first == "Content-Type")
		{
			os << "Content-Type: " << it->second << ";" << endl;
			os << "\tboundary=\"----=_NextPart_" << mp->getNumber() << "\"" << endl;
		}
		else
		{
			os << it->first << ": " << it->second << endl;
		}
	}
	os << endl;
	if (mp)
	{
		mp->writeTo(os);
	}
	else
	{
		os << content;
	}
}

MimeMessage::~MimeMessage()
{
	for (int i = 0; i < recipients.size(); i++)
	{
		delete recipients[i];
	}
}

void MimeMessage::setFrom(const char* addr)
{
	__require__(addr);
	from = addr;
	setHeader("From", addr);
}

void MimeMessage::addRecipient(RecipientType type, const char* addr)
{
	__require__(addr);
	char* tmp = new char[strlen(addr) + 1];
	strcpy(tmp, addr);
	recipients.push_back(tmp);
	string headerName;
	switch (type)
	{
		case RCPT_TO:
			headerName = "To";
			break;
		case RCPT_CC:
			headerName = "Cc";
			break;
		case RCPT_BCC:
			headerName = "Bcc";
			break;
		default:
			__unreachable__("Invalid recipient type");
	}
	string& header = headers[headerName];
	header += ", ";
	header += addr;
}

void TextHtmlPart::writeTo(ostream& os) const
{
	os << "Content-Type: text/html; charset=\"windows-1251\"" << endl;
	os << "Content-Transfer-Encoding: 8bit" << endl;
	os << endl;
	os << "<!doctype html public \"-//w3c//dtd html 4.0 transitional//en\">" << endl;
	os << "<html>" << endl;
	os << "<head>" << endl;
	os << "<meta http-equiv=\"content-type\" content=text/html; charset=\"windows-1251\">" << endl;
	os << "</head>" << endl;
	os << "<body bgcolor=#ffffff>" << endl;
	os << "<div><strong>bold</strong></div>" << endl;
	os << "<div><em>italic</em></div>" << endl;
	os << "<div><u>underline</u></div>" << endl;
	os << "<div><font color=#ff0000>red</font></div>" << endl;
	os << "</body>" << endl;
	os << "</html>" << endl;
}

void AttachmentPart::writeTo(ostream& os) const
{
	os << "Content-Type: image/gif; name=\"home.gif\"" << endl;
	os << "Content-Transfer-Encoding: base64" << endl;
	os << "Content-Disposition: attachment; filename=\"home.gif\"" << endl;
	os << endl;
	os << "R0lGODlhHgAOAKIAADtin/j6/8LO4G2Kt3aRvP///8bR4/4BAiH5BAUUAAcALAAAAAAeAA4AAAM+" << endl;
	os << "WLrcZi3K+QB4YWpnlRVbaFzMCIXSaK3siTLX9czxK5HLaE/kMBS6XYREIAABwiGkeEw2iMag08Na" << endl;
	os << "Ta/YTQIAOw==" << endl;
}

}
}
}

