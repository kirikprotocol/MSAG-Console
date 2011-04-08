/* $Id$ */

#include <cstdio>
#include "SerialBuffer.h"
#include "core/synchronization/Mutex.hpp"
#include "scag/util/io/HexDump.h"

/*
namespace {

const unsigned digitlen = 5;
smsc::core::synchronization::Mutex digitmtx;

const std::string& digitstring()
{
    static bool done = false;
    static std::string ds;
    if ( ! done ) {
        smsc::core::synchronization::MutexGuard mg(digitmtx);
        if ( ! done ) {
            ds.reserve(256*digitlen+1);
            char buf[10];
            for ( unsigned i = 0; i < 256; ++i ) {
                std::sprintf( buf, "%02x(%c)", i, (i>32 && i<128 ? i : '*' ));
                ds.append(buf);
            }
            done = true;
        }
    }
    return ds;
}

void bufdump( std::string& out, const char* inbuf, unsigned insize )
{
    out.reserve( out.size() + insize*digitlen + 10 );
    const char* digits = digitstring().c_str();
    for ( ; insize-- > 0; ++inbuf ) {
        out.append( digits + (static_cast<unsigned char>(*inbuf)*digitlen), digitlen );
    }
}

}
 */

namespace scag {
namespace util {
namespace storage {

std::string SerialBuffer::toString()
{
    HexDump hd;
    HexDump::string_type str;
    str.reserve(hd.hexdumpsize(length())+hd.strdumpsize(length())+10);
    hd.hexdump(str,c_ptr(),length());
    hd.strdump(str,c_ptr(),length());
    // bufdump( str, c_ptr(), length() );
    return hd.c_str(str);
}


void SerialBuffer::ReadString(std::string &str)
{
    uint16_t len;
    char scb[255];

    len = ReadInt16();

    if(getPos() + len > length())
        throw SerialBufferOutOfBounds();

    str.clear();
    while(len > 254)
    {
        blkread(scb, 254);
        str.append(scb, 254);
        len -= 254;
    }
    if(len)
    {
        blkread(scb, len);
        str.append(scb, len);
    }
}

void SerialBuffer::ReadString(std::wstring &str)
{
    uint16_t len, i; //, c;
    wchar_t scb[255];

    len = ReadInt16();

    if(getPos() + len > length())
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
    if(getPos() + count > length())
        throw SerialBufferOutOfBounds();

	blkread((char*)dst, count);
}

void SerialBuffer::Append(const char* data,int count)
{
    blkwrite(data, count);
}

void SerialBuffer::Empty()
{
    setLength(0);
    setPos(0);
}

uint32_t SerialBuffer::ReadInt32()
{
    if(int(getPos() + sizeof(uint32_t)) > length())
        throw SerialBufferOutOfBounds();

	uint32_t i;
	blkread((char*)&i, sizeof(i));
    return ntohl(i);
}

uint16_t SerialBuffer::ReadInt16()
{
    if(int(getPos() + sizeof(uint16_t)) > length())
        throw SerialBufferOutOfBounds();

	uint16_t i;
	blkread((char*)&i, sizeof(i));
    return ntohs(i);
}

uint8_t SerialBuffer::ReadInt8()
{
    if(int(getPos() + sizeof(uint8_t)) > length())
        throw SerialBufferOutOfBounds();

    uint8_t i;
	blkread((char*)&i, sizeof(i));
    return i;
}

void SerialBuffer::WriteInt32(uint32_t i)
{
    i = htonl(i);
    blkwrite((char*)&i, sizeof(i));
}

void SerialBuffer::WriteInt16(uint16_t i)
{
    i = htons(i);
    blkwrite((char*)&i, sizeof(i));
}

void SerialBuffer::WriteInt8(uint8_t i)
{
    blkwrite((char*)&i, sizeof(i));
}

void SerialBuffer::WriteString(const char *str)
{
    uint16_t len = ::strlen(str);
    WriteInt16(len);
    blkwrite(str, len);
}

void SerialBuffer::WriteString(const wchar_t *str)
{
    uint16_t len = std::wcslen(str);
    WriteInt16(len);
    while(len--)
        WriteInt16((uint16_t)*(str++));
}

} // namespace storage
} // namespace util
} // namespace scag

