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
			rand_char(length, buf, RAND_LAT_NUM + RAND_SYM);
			break;
		case UCS2:
			{
				int len = length / 2;
				char msg[len + 1];
				rand_char(len, msg, RAND_LAT_NUM + RAND_RUS + RAND_SYM);
				short* _buf = (short*) buf;
				length = ConvertMultibyteToUCS2(msg, len,
					_buf, length, CONV_ENCODING_CP1251);
				if (!hostByteOrder)
				{
					//���������� ������� �������
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
	if (!udhi)
	{
		return rand_text(length, dataCoding, hostByteOrder);
	}
	int headerLen = rand0(length - 1);
	__require__(headerLen >= 0);
	auto_ptr<uint8_t> header = rand_uint8_t(headerLen);
	int msgLen = length - headerLen - 1;
	auto_ptr<char> msg = rand_text(msgLen, dataCoding, hostByteOrder);
	__require__(msgLen >= 0);
	__require__(headerLen + msgLen + 1 <= length);
	length = headerLen + msgLen + 1;
	char* buf = new char[length];
	*buf = (unsigned char) headerLen;
	memcpy(buf + 1, header.get(), headerLen);
	memcpy(buf + headerLen + 1, msg.get(), msgLen);
	return auto_ptr<char>(buf);
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
					//���������� ������� �������
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
					//�������������� �� �������� ��������
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
				char buf[3 * text.length() + 1]; //� -> sch
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

vector<int> compare(bool udhi1, uint8_t dc1, const char* str1, int len1,
	bool udhi2, uint8_t dc2, const char* str2, int len2, bool hostByteOrder)
{
	vector<int> res;
	//udhi
	if (udhi1 != udhi2)
	{
		res.push_back(1);
		return res;
	}
	//���������� ��������� dc1
	switch (dc1)
	{
		case DEFAULT:
		case UCS2:
		case BINARY:
			break;
		default:
			res.push_back(2);
			return res;
	}
	//���������� ��������� dc2
	switch (dc2)
	{
		case DEFAULT:
		case UCS2:
		case BINARY:
			break;
		default:
			res.push_back(3);
			return res;
	}
	//��� ������
	if (!len1 && len2)
	{
		res.push_back(4);
		return res;
	}
	if (len1 && !len2)
	{
		res.push_back(5);
		return res;
	}
	if (!len1 && !len2)
	{
		return res;
	}
	//��������
	__require__(str1 && str2);
	if (dc1 == dc2)
	{
		if (len1 != len2)
		{
			res.push_back(6);
		}
		else if (memcmp(str1, str2, len1))
		{
			res.push_back(7);
		}
	}
	//DEFAULT -> UCS2 �� ������
	//UCS2 -> DEFAULT (� ��������)
	else if (dc1 == UCS2 && dc2 == DEFAULT)
	{
		char mbBuf[len1 + 1];
		int mbLen;
		int udhLen = udhi1 ? 1 + (unsigned char) *str1 : 0;
		//udh �������� ��� ���������
		if (udhLen)
		{
			if (udhLen > len2)
			{
				res.push_back(8);
			}
			else if (memcmp(str1, str2, udhLen))
			{
				res.push_back(9);
			}
		}
		//��������� �����������������
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
		char transBuf[(int) (len1 * 1.5) + 1]; //� -> sch
		int transLen = Transliterate(mbBuf, mbLen,
			CONV_ENCODING_CP1251, transBuf, sizeof(transBuf));
		if (transLen + udhLen != len2)
		{
			res.push_back(10);
		}
		else if (memcmp(transBuf, str2 + udhLen, transLen))
		{
			res.push_back(11);
			//__trace2__("transBuf: %s\nstr2: %s", transBuf, str2);
		}
	}
	else
	{
		res.push_back(12);
	}
	return res;
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
	static int tolerance = 3; //������
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
