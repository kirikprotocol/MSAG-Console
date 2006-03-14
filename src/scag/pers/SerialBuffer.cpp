/* $Id$ */

#include "SerialBuffer.h"

namespace scag{ namespace pers{

using namespace std;

using namespace smsc::core::buffers;

string SerialBuffer::toString()
{
	string str;
	int i = 0, j = GetPos();
	char buf[10];

	uint8_t b;
	SetPos(0);
	while(i++ < j)
	{
		Read((char*)&b, 1);
		sprintf(buf, "%02x(%c)", (int)b, (b > 32 && b < 128 ? b : '*'));
		str += buf;
	}
	SetPos(j);
	return str;
}

void SerialBuffer::ReadString(string &str)
{
	uint16_t len;
	char scb[255];

	Read((char*)&len, sizeof(len));

	if(pos + len > size)
		throw SerialBufferOutOfBounds();

	str[0] = 0;
	while(len > 254)
	{
		Read(scb, 254);
		scb[254] = 0;
		str += scb;
		len -= 254;
	}
	if(len)
	{
		Read((char*)scb, len);
		scb[len] = 0;
		str += scb;
	}
}

void SerialBuffer::ReadString(wstring &str)
{
	uint16_t len;
	wchar_t scb[255];

	Read((char*)&len, sizeof(len));

	if(pos + len > size)
		throw SerialBufferOutOfBounds();

	str[0] = 0;
	while(len > 254)
	{
		Read((char*)scb, sizeof(wchar_t) * 254);
		scb[254] = 0;
		str += scb;
		len -= 254;
	}
	if(len)
	{
		Read((char*)scb, sizeof(wchar_t) * len);
		scb[len] = 0;
		str += scb;
	}
}

void SerialBuffer::Read(char* dst, int count)
{
	if(pos + count > size)
		throw SerialBufferOutOfBounds();

	_SerialBuffer::Read(dst, count);
}

void SerialBuffer::Append(const char* data,int count)
{
	if(pos + count > size)
		size = pos + count;

	_SerialBuffer::Append(data, count);
}

void SerialBuffer::Empty()
{
	size = 0;
	SetPos(0);
}

uint32_t SerialBuffer::GetSize()
{
	return size;
}

}}
