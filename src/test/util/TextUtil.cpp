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
using namespace smsc::smpp::DataCoding;

string& mixedCase(string& str)
{
	for (int i = 0; i < str.length(); i++)
	{
		str[i] = rand0(1) ? toupper(str[i]) : tolower(str[i]);
	}
	return str;
}

auto_ptr<char> rand_text(int& length, uint8_t dataCoding, bool hostByteOrder)
{
	char* buf = new char[length + 1];
	rand_text(length, buf, dataCoding, hostByteOrder);
	return auto_ptr<char>(buf);
}

void rand_text(int& length, char* buf, uint8_t dataCoding, bool hostByteOrder)
{
	switch (dataCoding)
	{
		case DEFAULT:
			rand_char(length, buf, RAND_LAT_NUM + RAND_SYM + RAND_WS);
			break;
		case SMSC7BIT:
			{
				int ext = 0;
				char msg[length + 1];
				rand_char(length, msg, RAND_LAT_NUM + RAND_SYM + RAND_WS);
				for (int i = 0; i < length; i++)
				{
					switch (msg[i])
					{
						case '|':
						case '^':
						case '{':
						case '}':
						case '[':
						case ']':
						case '~':
						case '\\':
							ext++;
							break;
					}
				}
				length = ConvertLatin1ToSMSC7Bit(msg, length - ext, buf);
			}
			break;
		case UCS2:
			{
				int len = length / 2;
				char msg[len + 1];
				rand_char(len, msg, RAND_LAT_NUM + RAND_RUS + RAND_SYM + RAND_WS);
				short* _buf = (short*) buf;
				length = ConvertMultibyteToUCS2(msg, len,
					_buf, length, CONV_ENCODING_CP1251);
				__require__(length % 2 == 0);
				if (!hostByteOrder)
				{
					//установить сетевой порядок
					for (int i = 0; i < length / 2; i++)
					{
						*(_buf + i) = htons(*(_buf + i));
					}
				}
			}
			break;
		case BINARY:
			rand_uint8_t(length, (uint8_t*) buf);
			break;
		default:
			__unreachable__("Invalid dataCoding");
	}
}

auto_ptr<char> rand_text2(int& length, uint8_t dataCoding, bool udhi,
	bool hostByteOrder)
{
	char* buf = new char[length + 1];
	rand_text2(length, buf, dataCoding, udhi, hostByteOrder);
	return auto_ptr<char>(buf);
}

void rand_text2(int& length, char* buf, uint8_t dataCoding, bool udhi,
	bool hostByteOrder)
{
	if (!udhi)
	{
		rand_text(length, buf, dataCoding, hostByteOrder);
		__trace2__("rand_text2(): udhi = false, length = %d, dataCoding = %d, byteOrder = %s",
			length, (int) dataCoding, hostByteOrder ? "host" : "network");
		return;
	}
	__require__(length >= 3);
	//udh (ie - infrmational element)
	int udhLen = rand2(2, min(length - 1, 255));
	int ieLen = udhLen - 2;
	auto_ptr<uint8_t> ie = rand_uint8_t(ieLen);
	*buf = (unsigned char) udhLen;
	*(buf + 1) = 0x12; //Variable Picture
	*(buf + 2) = (unsigned char) ieLen;
	memcpy(buf + 3, ie.get(), ieLen);
	//text or binary data
	int msgLen = length - udhLen - 1;
	auto_ptr<char> msg = rand_text(msgLen, dataCoding, hostByteOrder);
	__require__(msgLen >= 0);
	__require__(udhLen + msgLen + 1 <= length);
	length = udhLen + msgLen + 1;
	memcpy(buf + udhLen + 1, msg.get(), msgLen);
	__trace2__("rand_text2(): udhi = true, length = %d (udhLen = %d, msgLen = %d), dataCoding = %d, byteOrder = %s",
		length, udhLen, msgLen, (int) dataCoding, hostByteOrder ? "host" : "network");
}

auto_ptr<char> encode(const string& text, uint8_t dataCoding, int& msgLen,
	bool hostByteOrder)
{
	char* msg;
	switch (dataCoding)
	{
		case DEFAULT:
		case BINARY:
			msgLen = text.length();
			msg = new char[msgLen];
			memcpy(msg, text.c_str(), msgLen);
			//msgLen = ConvertTextTo7Bit(text.c_str(), text.length(),
			//	msg, len, CONV_ENCODING_CP1251);
			break;
		case UCS2:
			{
				int len = text.length() * 2 + 1;
				msg = new char[len];
				short* _msg = (short*) msg;
				msgLen = ConvertMultibyteToUCS2(text.c_str(), text.length(),
					_msg, len, CONV_ENCODING_CP1251);
				if (!hostByteOrder && htons(0x1234) != 0x1234)
				{
					//установить сетевой порядок
					for (int i = 0; i < msgLen / 2; i++)
					{
						*(_msg + i) = htons(*(_msg + i));
					}
				}
			}
			break;
		case SMSC7BIT:
			{
				int len = 2 * text.length() + 1;
				msg = new char[len];
				msgLen = ConvertLatin1ToSMSC7Bit(text.c_str(), text.length(), msg);
			}
			break;
		default:
			__unreachable__("Invalid dataCoding");
	}
	return auto_ptr<char>(msg);
}

//todo: add udhi
const string decode(const char* text, int len, uint8_t dataCoding,
	bool hostByteOrder)
{
	switch (dataCoding)
	{
		case DEFAULT:
			//bufLen = Convert7BitToText(text, len, buf, sizeof(buf));
			return string(text, len);
		case UCS2:
			{
				char buf[len + 1];
				int bufLen;
				if (hostByteOrder || ntohs(0x1234) == 0x1234)
				{
					bufLen = ConvertUCS2ToMultibyte((const short*) text, len,
						buf, sizeof(buf), CONV_ENCODING_CP1251);
				}
				else
				{
					//перекодировать из сетевого порядока
					short tmp[len / 2];
					const short* _text = (const short*) text;
					for (int i = 0; i < len / 2; i++)
					{
						tmp[i] = ntohs(*(_text + i));
					}
					bufLen = ConvertUCS2ToMultibyte(tmp, sizeof(tmp),
						buf, sizeof(buf), CONV_ENCODING_CP1251);
				}
				return string(buf, bufLen);
			}
		case SMSC7BIT:
			{
				char buf[len];
				int bufLen = ConvertSMSC7BitToLatin1(text, len, buf);
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
				char buf[3 * text.length() + 1]; //щ -> sch
				int bufLen = Transliterate(text.c_str(), text.length(),
					CONV_ENCODING_CP1251, buf, sizeof(buf));
				return make_pair(string(buf, bufLen), DEFAULT);
			}
			else
			{
				return make_pair(text, DEFAULT);
			}
		case ProfileCharsetOptions::Ucs2:
			if (hasHighBit(text.c_str(), text.length()))
			{
				return make_pair(text, UCS2);
			}
			else
			{
				return make_pair(text, DEFAULT);
			}
		default:
			__unreachable__("Invalid codepage");
	}
	//return ...;
}

void convert(bool udhi, uint8_t dc1, const char* str1, int len1,
	uint8_t& dc2, char* str2, int& len2, int profileCodePage, bool hostByteOrder)
{
	__require__(str1 && str2 && len2);
	if (dc1 == UCS2 && profileCodePage == ProfileCharsetOptions::Default)
	{
		dc2 = DEFAULT;
		//udh проходит без изменений
		int udhLen = udhi ? 1 + (unsigned char) *str1 : 0;
		__require__(len2 > udhLen);
		memcpy(str2, str1, udhLen);
		//остальное транслитерируется
		char mbBuf[len1 - udhLen + 1];
		int mbLen;
		if (hostByteOrder || ntohs(0x1234) == 0x1234)
		{
			int ucs2Len = len1 - udhLen;
			const short* usc2Buf = (const short*) (str1 + udhLen);
			mbLen = ConvertUCS2ToMultibyte(usc2Buf, ucs2Len,
				mbBuf, sizeof(mbBuf), CONV_ENCODING_CP1251);
		}
		else
		{
			int ucs2Len = (len1 - udhLen) / 2;
			short ucs2Buf[ucs2Len];
			const short* _str1 = (const short*) (str1 + udhLen);
			for (int i = 0; i < ucs2Len; i++)
			{
				ucs2Buf[i] = ntohs(*(_str1 + i));
			}
			mbLen = ConvertUCS2ToMultibyte(ucs2Buf, sizeof(ucs2Buf),
				mbBuf, sizeof(mbBuf), CONV_ENCODING_CP1251);
		}
		//char transBuf[(int) ((len1 - udhLen) * 1.5) + 1]; //щ -> sch
		int transLen = Transliterate(mbBuf, mbLen,
			CONV_ENCODING_CP1251, str2 + udhLen, len2 - udhLen);
		__require__(len2 > udhLen + transLen);
		len2 = udhLen + transLen;
	}
	else
	{
		dc2 = dc1;
		__require__(len2 >= len1);
		memcpy(str2, str1, len1);
		len2 = len1;
	}
}

uint8_t getDataCoding(int num)
{
	static const uint8_t dataCodings[] = {DEFAULT, UCS2 /*, BINARY*/};
	static const int dataCodingsLen = sizeof(dataCodings) / sizeof(*dataCodings);
	switch (num)
	{
		case RAND_TC:
			return dataCodings[rand0(dataCodingsLen - 1)];
		case ProfileCharsetOptions::Default:
			return DEFAULT;
		case ProfileCharsetOptions::Ucs2:
			return UCS2;
		default:
			__unreachable__("Invalid num");
	}
}

uint8_t getTextDataCoding(int num)
{
	static const uint8_t dataCodings[] = {DEFAULT, UCS2};
	static const int dataCodingsLen = sizeof(dataCodings) / sizeof(*dataCodings);
	switch (num)
	{
		case RAND_TC:
			return dataCodings[rand0(dataCodingsLen - 1)];
		case ProfileCharsetOptions::Default:
			return DEFAULT;
		case ProfileCharsetOptions::Ucs2:
			return UCS2;
		default:
			__unreachable__("Invalid num");
	}
}

int getMaxChars(uint8_t dataCoding)
{
	switch (dataCoding)
	{
		case DEFAULT:
			return 160;
		case UCS2:
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
