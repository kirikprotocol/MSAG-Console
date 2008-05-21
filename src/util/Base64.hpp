#include <string>

namespace smsc{
namespace util{

static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

inline void encodeblock64( unsigned char in[3], unsigned char out[4], size_t len )
{
    out[0] = cb64[ in[0] >> 2 ];
    out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
    out[2] = (unsigned char) (len > 1 ? cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] : '=');
    out[3] = (unsigned char) (len > 2 ? cb64[ in[2] & 0x3f ] : '=');
}

inline std::string encode64( const std::string& str)
{
  unsigned char in[3], out[5]={0,};
  size_t i=0,len;
  std::string outstr;

  while(i<str.length())
  {
    if(str.length()-i>=3)
    {
      in[0]=str[i];
      in[1]=str[i+1];
      in[2]=str[i+2];
      len=3;
    }else
    {
      in[2]=in[1]=in[0]=0;
      len=str.length()-i;
      switch(len)
      {
        case 2:in[1]=str[i+1];
        case 1:in[0]=str[i];
      }
    }
    encodeblock64(in,out,len);
    outstr+=(char*)out;
  i+=3;
  }
  return outstr;
}

inline void decodeblock64( unsigned char in[4], unsigned char out[3] )
{
  out[ 0 ] = (unsigned char ) (in[0] << 2 | in[1] >> 4);
  out[ 1 ] = (unsigned char ) (in[1] << 4 | in[2] >> 2);
  out[ 2 ] = (unsigned char ) (((in[2] << 6) & 0xc0) | in[3]);
}

inline std::string decode64(const std::string& str)
{
  unsigned char in[4], out[4]={0,}, v;
  int i, len,pos=0;
  std::string outstr;

  while( pos<str.length() )
  {
    for( len = 0, i = 0; i < 4 && pos<str.length(); i++ )
    {
      v = 0;
      while( pos<=str.length() && v == 0 )
      {
        v = (unsigned char) str.c_str()[pos++];
        v = (unsigned char) ((v < 43 || v > 122) ? 0 : cd64[ v - 43 ]);
        if( v )
        {
          v = (unsigned char) ((v == '$') ? 0 : v - 61);
        }
      }
      if( pos<=str.length() )
      {
        len++;
        if( v )
        {
          in[ i ] = (unsigned char) (v - 1);
        }
      }
      else
      {
        in[i] = 0;
      }
    }
    if( len )
    {
      decodeblock64( in, out );
      printf("len=%d; out=%s\n",len,out);
      outstr.append((char*)out,len-1);
    }
  }
  return outstr;
}

}//util
}//smsc
