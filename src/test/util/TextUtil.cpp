#include "TextUtil.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace util {

using namespace std;

/*
std::auto_ptr<char> rand_text(int length, uint8_t dataCoding)
{
	switch (dataCoding)
	{
		case DATA_CODING_SMSC_DEFAULT:
			auto_ptr<char> str = rand_char(length, RAND_LAT + RAND_RUS + RAND_NUM + RAND_SYM);
			break;
		case DATA_CODING_UCS2:
			rand_char(length / 2, RAND_LAT + RAND_RUS + RAND_NUM + RAND_SYM)
			break;
		default:
			__unreachable__("Invalid dataCoding");
	}
	return text;



	char* buf = new char[length + 1];
	rand_text(length, buf, dataCoding);
	return auto_ptr<char>(buf);
}

void rand_text(int& length, char* buf, uint8_t dataCoding)
{
	__require__(dataCoding == DATA_CODING_SMSC_DEFAULT);
	rand_char(length, buf);

	int len = 0;
	char tmp[12];
	while (len <= length)
	{
		switch ()
		{
			case 1:
				rand_char(rand1(8), tmp, RAND_LAT);
				break;
			case 2:
				RAND_RUS
				break;
			case 3:
				RAND_NUM
				break;
			case 4:
				RAND_LAT + RAND_RUS + RAND_NUM + RAND_SYM
			default:
				RAND_SYM
		}
		switch (dataCoding)
		{
			case DATA_CODING_SMSC_DEFAULT:
				auto_ptr<char> str = rand_char(length, RAND_LAT + RAND_RUS + RAND_NUM + RAND_SYM);
				break;
			case DATA_CODING_UCS2:
				rand_char(length / 2, RAND_LAT + RAND_RUS + RAND_NUM + RAND_SYM)
				break;
			default:
				__unreachable__("Invalid dataCoding");
		}
	}
}
*/

auto_ptr<char> encode(const string& text, uint8_t dataCoding, int& msgLen)
{
	int len = text.length() * 2;
	char* msg = new char[len];
	switch (dataCoding)
	{
		case DATA_CODING_SMSC_DEFAULT:
			msgLen = ConvertTextTo7Bit(text.c_str(), text.length(),
				msg, len, CONV_ENCODING_CP1251);
			break;
		case DATA_CODING_UCS2:
			msgLen = ConvertMultibyteToUCS2(text.c_str(), text.length(),
				(short*) msg, len / sizeof(short), CONV_ENCODING_CP1251);
			break;
		default:
			__unreachable__("Invalid dataCoding");
	}
	return auto_ptr<char>(msg);
}

const string decode(const char* text, int len, uint8_t dataCoding)
{
	int bufLen;
	char buf[len + 1];
	switch (dataCoding)
	{
		case DATA_CODING_SMSC_DEFAULT:
			bufLen = Convert7BitToText(text, len, buf, sizeof(buf));
			return buf;
		case DATA_CODING_UCS2:
			__require__(len % 2 == 0);
			bufLen = ConvertUCS2ToMultibyte((const short*) text, len / 2,
				buf, sizeof(buf), CONV_ENCODING_CP1251);
			return buf;
		default:
			__unreachable__("Invalid dataCoding");
	}
	//return "";
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
		short ucs2Buf[len1];
		int ucs2Len = Convert7BitToUCS2(str1, len1, ucs2Buf, sizeof(ucs2Buf));
		if (ucs2Len != len2)
		{
			res.push_back(5);
		}
		else if (memcmp(ucs2Buf, str2, len2))
		{
			res.push_back(6);
		}
	}
	//DATA_CODING_UCS2 -> DATA_CODING_SMSC_DEFAULT (в транслит)
	else if (dc1 == DATA_CODING_UCS2 && dc2 == DATA_CODING_SMSC_DEFAULT)
	{
		__require__(len1 % 2 == 0);
		char bit7Buf[len1];
		int bit7Len = ConvertUCS2To7Bit((const short*) str1, len1 / 2,
			bit7Buf, sizeof(bit7Buf));
		if (bit7Len != len2)
		{
			res.push_back(7);
		}
		else if (memcmp(bit7Buf, str2, len2))
		{
			res.push_back(8);
		}
	}
	else
	{
		__unreachable__("Invalid data codings");
	}
	return res;
}

uint8_t getDataCoding(int num)
{
	const uint8_t dataCodings[] = {DATA_CODING_SMSC_DEFAULT, DATA_CODING_UCS2};
	switch (num)
	{
		case RAND_TC:
			return dataCodings[rand0(1)];
		case 1:
			return DATA_CODING_SMSC_DEFAULT;
		case 2:
			return DATA_CODING_UCS2;
		default:
			__unreachable__("Invalid num");
	}
}

}
}
}
