#include "TextUtil.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace util {

using namespace std;

std::auto_ptr<char> rand_text(int length, uint8_t dataCoding)
{
	char* buf = new char[length + 1];
	rand_text(length, buf, dataCoding);
	return auto_ptr<char>(buf);
}

void rand_text(int length, char* buf, uint8_t dataCoding)
{
	__require__(dataCoding == DATA_CODING_SMSC_DEFAULT);
	rand_char(length, buf);
}

const string encode(const char* text, uint8_t dataCoding)
{
	return text;
}

const string decode(const char* text, uint8_t dataCoding)
{
	return text;
}

vector<int> compare(uint8_t dc1, const char* str1, int len1,
	uint8_t dc2, const char* str2, int len2)
{
	vector<int> res;
	//допустимая кодировка dc1
	switch (dc1)
	{
		case DATA_CODING_SMSC_DEFAULT:
		case DATA_CODING_UCS2:
			break;
		default:
			res.push_back(1);
			return res;
	}
	//допустимая кодировка dc2
	switch (dc2)
	{
		case DATA_CODING_SMSC_DEFAULT:
		case DATA_CODING_UCS2:
			break;
		default:
			res.push_back(2);
			return res;
	}
	if (dc1 == dc2)
	{
		if (len1 != len2)
		{
			res.push_back(3);
		}
		else if (memcmp(str1, str2, len1))
		{
			res.push_back(4);
		}
	}
	//DATA_CODING_SMSC_DEFAULT -> DATA_CODING_UCS2
	else if (dc1 == DATA_CODING_SMSC_DEFAULT && dc2 == DATA_CODING_UCS2)
	{
		res.push_back(5);
	}
	//DATA_CODING_UCS2 -> DATA_CODING_SMSC_DEFAULT (в транслит)
	else if (dc1 == DATA_CODING_UCS2 && dc2 == DATA_CODING_SMSC_DEFAULT)
	{
		res.push_back(6);
	}
	else
	{
		__unreachable__("Invalid data codings");
	}
	return res;
}

}
}
}
