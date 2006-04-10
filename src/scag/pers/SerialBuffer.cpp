/* $Id$ */

#include "SerialBuffer.h"

namespace scag{ namespace pers{

using namespace smsc::core::buffers;

std::string SerialBuffer::toString()
{
    std::string str;
    int i = 0, j = GetSize(), k = GetPos();
    char buf[10];

    uint8_t b;
    SetPos(0);
    while(i++ < j)
    {
        b = ReadInt8();
        sprintf(buf, "%02x(%c)", (int)b, (b > 32 && b < 128 ? b : '*'));
        str += buf;
    }
    SetPos(k);
    return str;
}

void SerialBuffer::ReadString(std::string &str)
{
    uint16_t len;
    char scb[255];

    len = ReadInt16();

    if(pos + len > size)
        throw SerialBufferOutOfBounds();

    str = "";
    while(len > 254)
    {
        Read(scb, 254);
        scb[254] = 0;
        str += scb;
        len -= 254;
    }
    if(len)
    {
        Read(scb, len);
        scb[len] = 0;
        str += scb;
    }
}

void SerialBuffer::ReadString(std::wstring &str)
{
    uint16_t len, i, c;
    wchar_t scb[255];

    len = ReadInt16();

    if(pos + len > size)
        throw SerialBufferOutOfBounds();

    str = L"";
    while(len > 254)
    {
        i = 0;
        while(i < 254)
            scb[i++] = (wchar_t)ReadInt16();

        scb[254] = 0;
        str += scb;
        len -= 254;
    }
    if(len)
    {
        i = 0;
        while(i < len)
            scb[i++] = (wchar_t)ReadInt16();

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
    return ntohl(i);
}

uint16_t SerialBuffer::ReadInt16()
{
    uint16_t i;
    Read((char*)&i, sizeof(i));
    return ntohs(i);
}

uint8_t SerialBuffer::ReadInt8()
{
    uint8_t i;
    Read((char*)&i, sizeof(i));
    return i;
}

void SerialBuffer::WriteInt32(uint32_t i)
{
    i = htonl(i);
    Append((char*)&i, sizeof(i));
}

void SerialBuffer::WriteInt16(uint16_t i)
{
    i = htons(i);
    Append((char*)&i, sizeof(i));
}

void SerialBuffer::WriteInt8(uint8_t i)
{
    Append((char*)&i, sizeof(i));
}

void SerialBuffer::WriteString(const char *str)
{
    uint16_t len = std::strlen(str);
    WriteInt16(len);
    Append(str, len);
}

void SerialBuffer::WriteString(const wchar_t *str)
{
    uint16_t len = std::wcslen(str);
    WriteInt16(len);
    while(len--)
        WriteInt16((uint16_t)*(str++));
}

}}
