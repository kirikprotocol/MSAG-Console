#ifndef TEST_EMAIL_SME_MIME_MESSAGE
#define TEST_EMAIL_SME_MIME_MESSAGE

#include <map>
#include <vector>
#include <ostream>
#include <string>

namespace smsc {
namespace test {
namespace emailsme {

using std::map;
using std::vector;
using std::ostream;
using std::string;

class Part;
class Multipart
{
	int number;
	vector<Part*> parts;
public:
	Multipart();
	~Multipart();
	int getNumber() const { return number; }
	void addPart(Part* part) { parts.push_back(part); }
	void writeTo(ostream& os) const;
};

class Part
{
protected:
	typedef map<const string, string> Headers;
	Headers headers;
	string content;
	string contentType;
	const Multipart* mp;
public:
	Part() : mp(NULL) {}
	virtual ~Part();
	void setHeader(const char* name, const char* value);
	void setContent(const char* content);
	void setContent(const Multipart* mp);
	virtual void writeTo(ostream& os) const;
};

typedef enum
{
	RCPT_TO = 1,
	RCPT_CC = 2,
	RCPT_BCC = 3
} RecipientType;

class MimeMessage : public Part
{
	string from;
	vector<const char*> recipients;
public:
	~MimeMessage();
	const char* getFrom() const { return from.c_str(); }
	const vector<const char*>& getRecipients() const { return recipients; }
	void setFrom(const char* addr);
	void addRecipient(RecipientType type, const char* addr);
};

class TextHtmlPart : public Part
{
	virtual void writeTo(ostream& os) const;
};

class AttachmentPart : public Part
{
	virtual void writeTo(ostream& os) const;
};

}
}
}

#endif /* TEST_EMAIL_SME_MIME_MESSAGE */

