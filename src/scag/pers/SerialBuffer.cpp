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
		b = ReadInt8();
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

	len = ReadInt16();

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

	len = ReadInt16();

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

uint32_t SerialBuffer::ReadInt32()
{
	uint32_t i;
	Read((char*)&i, sizeof(i));
	return i;
}

uint16_t SerialBuffer::ReadInt16()
{
	uint16_t i;
	Read((char*)&i, sizeof(i));
	return i;
}

uint8_t SerialBuffer::ReadInt8()
{
	uint8_t i;
	Read((char*)&i, sizeof(i));
	return i;
}

void SerialBuffer::WriteInt32(uint32_t i)
{
	Append((char*)&i, sizeof(i));
}

void SerialBuffer::WriteInt16(uint16_t i)
{
	Append((char*)&i, sizeof(i));
}

void SerialBuffer::WriteInt8(uint8_t i)
{
	Append((char*)&i, sizeof(i));
}

void SerialBuffer::WriteString(const char *str)
{
	uint16_t len = strlen(str);
	WriteInt16(len);
	Append(str, len);
}

void SerialBuffer::WriteString(const wchar_t *str)
{
	uint16_t len = wcslen(str);
	WriteInt16(len);
	Append((char*)str, sizeof(wchar_t) * len);
}

}}
