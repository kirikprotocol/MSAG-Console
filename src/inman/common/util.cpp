static char const ident[] = "$Id$";

#include <assert.h>
#include <vector>

#include "inman/common/util.hpp"

namespace smsc {
namespace inman {
namespace common {


using namespace std;

void pack_addr(unsigned char* dst, const char* src, int len)
{
  for( int i = 0; i < len; i++ )
  {
    if( i & 1 )
    {
      dst[i/2] |= ((src[i]-'0')<<4); // fill high semioctet
    }
    else
    {
      dst[i/2] = (src[i]-'0')&0x0f; // fill low semioctet
    }
  }
  if( len & 1 ) {
    dst[(len+1)/2-1] &= 0x0F;
  }
}


int format(std::string & fstr, const char* fmt, ...)
{
    char format_buffer[4096];

    va_list va;
    va_start(va, fmt);
#ifdef _WIN32
    int u = _vsnprintf
#else
    int u = vsnprintf
#endif
            (format_buffer, sizeof(format_buffer) - 1, fmt, va);
    va_end(va);
    format_buffer[u] = 0;
    fstr += format_buffer;
    return u;
}

std::string format(const char* fmt, ...)
{
    char format_buffer[4096];

    va_list va;
    va_start(va, fmt);
#ifdef _WIN32
    int u = _vsnprintf
#else
    int u = vsnprintf
#endif
            (format_buffer, sizeof(format_buffer) - 1, fmt, va);
    va_end(va);
    format_buffer[u] = 0;
    return std::string(format_buffer);
}


void unpack_addr(char* dst, unsigned char* src, int len)
{
  unsigned char sign;
  for (int i = 0; i < len; i++)
  {
    if (i & 1)
    {
      sign = (src[i/2] >> 4) & 0x0f;
    }
    else
    {
      sign = src[i/2] & 0x0f;
    }
    switch(sign)
    {
      case 0: case 1: case 2: case 3: case 4: case 5:
      case 6: case 7: case 8: case 9: dst[i] = sign + '0'; break;
      case 11: dst[i] = '*'; break;
      case 12: dst[i] = '#'; break;
      case 15: dst[i] =  0 ; break;
      default: dst[i] = '?'; break;
    }
  }
  dst[len] = 0;
}

void fillAddress(SCCP_ADDRESS_T* dst, const char *saddr, unsigned char ssn)
{
  unsigned len = strlen(saddr);
  dst->addrLen = 5 + (len + 1)/2;             // length in octets
  dst->addr[0] = 0x12;                        // SSN & GT indicator
  dst->addr[1] = ssn;                         // SSN
  dst->addr[2] = 0;                           // Translation Type
  dst->addr[3] = 0x10 | (len%2==0?0x02:0x01); // NP & GT coding
  dst->addr[4] = 0x04;                        // Nature of address

  for( int i = 0; i < len; i++ )
  {
    if( i & 1 ) {
      dst->addr[i/2+5] |= ((saddr[i]-'0')<<4);// fill high semioctet
    } else {
      dst->addr[i/2+5] = (saddr[i]-'0')&0x0F; // fill low semioctet
    }
  }

  if( len & 1 ) {
    dst->addr[(len+1)/2-1+5] &= 0x0F;
  }
}

void fillAddress( ADDRESS_BUF_T *dst, const char *saddr)
{
  unsigned len = strlen(saddr);
  dst->addrLen = 1 + (len + 1)/2;             // length in octets
  dst->addr[0] = 0x91;                        // TON,NPI

  for( int i = 0; i < len; i++ )
  {
    if( i & 1 ) {
      dst->addr[i/2+1] |= ((saddr[i]-'0')<<4);// fill high semioctet
    } else {
      dst->addr[i/2+1] = (saddr[i]-'0')&0x0F; // fill low semioctet
    }
  }

  if( len & 1 ) {
    dst->addr[(len+1)/2-1+1] |= 0xF0;
  }
}


int dump(std::string& fstr, unsigned short size, unsigned char* buff, bool ascii)
{
    int chars = 0;
    for (int i = 0; i < size; i++) {
        char tmp[32];
        unsigned char ch = buff[i];

        if ((ch < 32) || (ch > 127) || !ascii)
            chars += sprintf(tmp, "0x%02X ", ch);
        else 
            chars += sprintf(tmp, "'%c' ", ch);
        fstr += tmp;
    }
    if (size)
        fstr.erase(fstr.end()); //erase last space char
    return chars;
}

std::string dump(unsigned short size, unsigned char* buff, bool ascii)
{
    std::string reply;
    dump(reply, size, buff, ascii);
    return reply;
}


void dumpToLog(Logger* logger, int len, const unsigned char* buffer )
{
  if( !buffer )
  {
    smsc_log_debug( logger, "%s", "Buffer is NULL" );
    smsc_log_debug( logger, "" );
    return;
  }

  std::string row;
  char tmp[32];
  for(int addr = 0; addr < len; addr++ )
  {
    if(( addr & 0x0F ) == 0 )
    {
      if( !row.empty() ) smsc_log_debug( logger, "%s", row.c_str() );
      sprintf( tmp, "%04X: ", addr );
      row = tmp;
    }
    sprintf( tmp, " %02X", buffer[addr] );
    row += tmp;
  }
  if( len & 0x0F )
  {
    smsc_log_debug( logger, "%s", row.c_str() );
  }
  smsc_log_debug( logger, "Total: %d byte(s)", len );
  smsc_log_debug( logger, "" );
}

}//namespace common
}//namespace inman
}//namespace smsc

