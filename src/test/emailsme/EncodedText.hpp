#ifndef TEST_EMAIL_SME_ENCODED_TEXT
#define TEST_EMAIL_SME_ENCODED_TEXT

#include <map>
#include <string>
#include <sstream>

namespace smsc {
namespace test {
namespace emailsme {

using std::map;
using std::string;
using std::ostringstream;

typedef enum
{
	KOI8_U = 1,
	KOI8_R = 2,
	WINDOWS_1251 = 3,
	ISO_8859_5 = 4
} Charset;

typedef enum
{
	QUOTED_PRINTABLE = 1,
	BIT8 = 2,
	BIT7 = 3,
	BASE64 = 4,
	//unsupported
	BINARY = 5,
	X_TOKEN = 6
} TransferEncoding;

typedef enum
{
	LATIN = 1,
	LATIN2 = 2,
	RUS = 3,
	RUS2 = 4
} TextType;

struct TextTable : public map<const string, string>
{
	TextTable();
};

class EncodedText
{
	static TextTable t;
	const Charset charset;
	const TransferEncoding transferEncoding;
	ostringstream text;
	ostringstream encText;
public:
	EncodedText(Charset charset, TransferEncoding transferEncoding);
	const char* getCharset();
	const char* getTransferEncoding();
	const char* getText();
	const char* getEncodedText();
	int appendText(TextType type);
};

}
}
}

#endif /* TEST_EMAIL_SME_ENCODED_TEXT */

