#include "TextUtil.hpp"
#include "profiler/profiler.hpp"
#include "util/debug.h"
#include "util/smstext.h"

namespace smsc {
namespace test {
namespace util {

using namespace std;
using namespace smsc::profiler;
using namespace smsc::util;

auto_ptr<char> rand_text(int& length, uint8_t dataCoding)
{
	char* buf = new char[length + 1];
	rand_text(length, buf, dataCoding);
	return auto_ptr<char>(buf);
}

void rand_text(int& length, char* buf, uint8_t dataCoding)
{
	switch (dataCoding)
	{
		case DATA_CODING_SMSC_DEFAULT:
			for (int i = 0; i < length; i++)
			{
				buf[i] = rand0(127);
			}
			break;
		case DATA_CODING_UCS2:
			{
				char tmp[length / 2];
				rand_uint8_t(length / 2, (uint8_t*) tmp);
				int len = ConvertMultibyteToUCS2(tmp, sizeof(tmp),
					(short*) buf, length, CONV_ENCODING_CP1251);
				__require__(length/2 - len/2  == 0);
				length = len;
			}
			break;
		default:
			__unreachable__("Invalid dataCoding");
	}
}

auto_ptr<char> encode(const string& text, uint8_t dataCoding, int& msgLen)
{
	char* msg;
	switch (dataCoding)
	{
		case DATA_CODING_SMSC_DEFAULT:
			msgLen = text.length();
			msg = new char[msgLen];
			memcpy(msg, text.c_str(), msgLen);
			//msgLen = ConvertTextTo7Bit(text.c_str(), text.length(),
			//	msg, len, CONV_ENCODING_CP1251);
			break;
		case DATA_CODING_UCS2:
			{
				int len = text.length() * 2 + 1;
				msg = new char[len];
				msgLen = ConvertMultibyteToUCS2(text.c_str(), text.length(),
					(short*) msg, len, CONV_ENCODING_CP1251);
			}
			break;
		default:
			__unreachable__("Invalid dataCoding");
	}
	return auto_ptr<char>(msg);
}

const string decode(const char* text, int len, uint8_t dataCoding)
{
	switch (dataCoding)
	{
		case DATA_CODING_SMSC_DEFAULT:
			//bufLen = Convert7BitToText(text, len, buf, sizeof(buf));
			return string(text, len);
		case DATA_CODING_UCS2:
			{
				char buf[len + 1];
				int bufLen = ConvertUCS2ToMultibyte((const short*) text, len,
					buf, sizeof(buf), CONV_ENCODING_CP1251);
				return string(buf, bufLen);
			}
		default:
			__unreachable__("Invalid dataCoding");
	}
	//return "";
}

const pair<string, uint8_t> convert(const string& text, int profileCodePage)
{
	switch (profileCodePage)
	{
		case ProfileCharsetOptions::Default:
			if (hasHighBit(text.c_str(), text.length()))
			{
				char buf[2 * text.length()];
				int bufLen = Transliterate(text.c_str(), text.length(),
					CONV_ENCODING_CP1251, buf, sizeof(buf));
				return make_pair(string(buf, bufLen), DATA_CODING_SMSC_DEFAULT);
			}
			else
			{
				return make_pair(text, DATA_CODING_SMSC_DEFAULT);
			}
		case ProfileCharsetOptions::Ucs2:
			if (hasHighBit(text.c_str(), text.length()))
			{
				return make_pair(text, DATA_CODING_UCS2);
			}
			else
			{
				return make_pair(text, DATA_CODING_SMSC_DEFAULT);
			}
		default:
			__unreachable__("Invalid codepage");
	}
	//return ...;
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
		char ucs2Buf[2 * len1 + 1];
		//int ucs2Len = Convert7BitToUCS2(str1, len1, (short*) ucs2Buf, sizeof(ucs2Buf));
		int ucs2Len = ConvertMultibyteToUCS2(str1, len1, (short*) ucs2Buf,
			sizeof(ucs2Buf), CONV_ENCODING_CP1251);
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
		char defBuf[len1 + 1];
		char transBuf[len1 + 1];
		int defLen = ConvertUCS2ToMultibyte((const short*) str1, len1,
			defBuf, sizeof(defBuf), CONV_ENCODING_CP1251);
		int transLen = Transliterate(defBuf, defLen,
			CONV_ENCODING_CP1251, transBuf, sizeof(transBuf));
		if (transLen != len2)
		{
			res.push_back(7);
		}
		else if (memcmp(transBuf, str2, len2))
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
		case ProfileCharsetOptions::Default:
			return DATA_CODING_SMSC_DEFAULT;
		case ProfileCharsetOptions::Ucs2:
			return DATA_CODING_UCS2;
		default:
			__unreachable__("Invalid num");
	}
}

int getMaxChars(uint8_t dataCoding)
{
	switch (dataCoding)
	{
		case DATA_CODING_SMSC_DEFAULT:
			return 160;
		case DATA_CODING_UCS2:
			return 70;
		default:
			__unreachable__("Invalid dataCoding");
	}
}

int findPos(const string& text, const string& expected,
	int segmentSize, bool& check)
{
	static int tolerance = 3; //допуск
	check = false;
	if (!expected.length() || expected.length() < text.length())
	{
		return string::npos;
	}
	int lastSeg = (expected.length() - 1) / segmentSize;
	for (int seg = 0; seg <= lastSeg; seg++)
	{
		for (int shift = 0; shift <= tolerance * seg; shift++)
		{
			int pos = seg * segmentSize - shift;
			if (!expected.compare(pos, text.length(), text))
			{
				if (seg == lastSeg)
				{
					check = (text.length() <= segmentSize) &&
						(pos + text.length() == expected.length());
				}
				else
				{
					check = (text.length() >= segmentSize - tolerance) &&
						(text.length() <= segmentSize);
				}
				return pos;
			}
		}
	}
	return string::npos;
}

}
}
}
