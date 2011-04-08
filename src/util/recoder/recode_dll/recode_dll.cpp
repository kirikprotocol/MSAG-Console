// recode_dll.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "../recode_dll.h"
#include "cp1251.hxx"
#include "koi8_r.hxx"
#include "8859_5.hxx"
#include "latin1.hxx"
#include "translit.hxx"
#include "7_8_bits.hxx"
#include <stdexcept>
#include <ctype.h>
using namespace std;


unsigned char* GetW2CRussionTable(ConvEncodingEnum encoding)
{
  switch ( encoding ){
  case CONV_ENCODING_CP1251:
    return w2c_Cp1251_table;
  case CONV_ENCODING_ANSI:
  case CONV_ENCODING_LATIN1:
    return w2c_Latin1_table;
  case CONV_ENCODING_KOI8R:
    return w2c_KOI8_R_table;
  case CONV_ENCODING_ISO8895_5:
    return w2c_8859_5_table;
  default:
    FatalError(__FILE__,__LINE__,"unsupported encoding %d",encoding);
    return NULL;
  }
}

unsigned short* GetC2WTable(ConvEncodingEnum encoding)
{
  switch ( encoding ){
  case CONV_ENCODING_CP1251:
    return c2w_Cp1251_table;
  case CONV_ENCODING_ANSI:
  case CONV_ENCODING_LATIN1:
    return c2w_Latin1_table;
  case CONV_ENCODING_KOI8R:
    return c2w_KOI8_R_table;
  case CONV_ENCODING_ISO8895_5:
    return c2w_8859_5_table;
  default:
    FatalError(__FILE__,__LINE__,"unsupported encoding %d",encoding);
    return NULL;
  }
}

unsigned char ConvertW2C(unsigned short val, ConvEncodingEnum encoding )
{
  switch( val&0xff00 ){
  case 0x0400: // Russion
    {
      unsigned char* w2c_convert = GetW2CRussionTable(encoding);
      if ( !w2c_convert ) return '?';
      return w2c_convert[val&0xff];
    }
  case 0x2100:
    if((val&0xff)==0x16)
    {
      switch(encoding)
      {
        case CONV_ENCODING_CP1251:return '¹';
        case CONV_ENCODING_KOI8R:return '¿';
        case CONV_ENCODING_ANSI:return'î';
        default:return '?';
      }
    }else return '?';
  case 0x0: //ANSI
    return (unsigned char)(val&0xff);
  default: // Unknown
    return '?';
  }
}

unsigned short ConvertC2W(unsigned char val, ConvEncodingEnum encoding )
{
  unsigned short* c2w_convert = GetC2WTable(encoding);
  if ( !c2w_convert ) return (0x0000|'?');
  return c2w_convert[val];
}

#define CONV_PREVIOUS_IS_CAPITAL  1
#define CONV_NEXT_IS_CAPITAL 2

bool isxalpha(unsigned char chr)
{
  return isalpha(chr) || chr>=192 || chr==168 || chr==168+16;
}
bool isxupper(unsigned char chr)
{
  return (chr>=192 && chr<224) || chr==168;
}

const unsigned char* Translit(unsigned char ch, ConvEncodingEnum encoding, unsigned flags, bool* is_capital) {
  if ( encoding == CONV_ENCODING_ANSI || encoding == CONV_ENCODING_LATIN1 ){
    static unsigned char chh[]="\0";
    chh[0] = ch;
    return chh;
  }if ( encoding != CONV_ENCODING_CP1251 ){
    ch = ConvertW2C(ConvertC2W(ch,encoding),CONV_ENCODING_CP1251);
  }
  if ( ch >= 192 && ch < 224 ) *is_capital = true; else *is_capital = false;
  const unsigned char* res = c2t_Cp1251_table[ch];
  if ( *res == '*' ){
    ++res;
    if ( flags&CONV_NEXT_IS_CAPITAL){
      return res;
    }else{
      return res+strlen((const char*)res)+1;
    }
  }else return res;
}

int Transliterate(const char* buf,size_t len,ConvEncodingEnum encoding,char *dest,size_t destlen)
{
  if(len==0)return 0;
  size_t j=0;
  const unsigned char* res;
  bool prev_is_capital,next_is_capital;
  if(len>1)
  {
    if((unsigned char)buf[1]>127)Translit((unsigned char)buf[1],encoding,0,&prev_is_capital);
    else if(buf[1]>='A' && buf[1]<='Z')prev_is_capital=true;
  }else
  {
    prev_is_capital=false;
  }
  for(int i=0;i<len;i++)
  {
    unsigned char ch=(unsigned char)buf[i];
    if(i<len-1)
    {
      next_is_capital=!isxalpha(buf[i+1]) || isxupper(buf[i+1]);
    }
    else next_is_capital=false;
    if(ch>127)
    {
      res=Translit(ch,encoding,
        (prev_is_capital?CONV_PREVIOUS_IS_CAPITAL:0) |
        (next_is_capital?CONV_NEXT_IS_CAPITAL:0)
        ,&prev_is_capital);
      for(;*res;res++)
      {
        dest[j]=*res;
        j++;
        if(j==destlen)return -1;
      }
    }else
    {
      dest[j]=ch;
      j++;
      if(j==destlen)return -1;
    }
  }
  dest[j]=0;
  return (int)j;
}

int RECODE_DECL ConvertUCS2ToMultibyte(const short* ucs2, size_t ucs2buff_size,char* text, size_t textbuff_size, ConvEncodingEnum encoding)
{
  size_t i;
  short c;
  for ( i = 0; i < ucs2buff_size/2; ++i )
  {
    ensure(textbuff_size>i);
    memcpy(&c,ucs2+i,2);
    if ((c>>8) == 0 ) {
      text[i] = (unsigned char)(c&0xff);
    }else{
      text[i] = ConvertW2C(c,encoding);
    }
  }
  return (int)i;
}

int RECODE_DECL ConvertMultibyteToUCS2(const char* text, size_t textbuff_size,short* ucs2, size_t ucs2buff_size, ConvEncodingEnum encoding)
{
  size_t i;
  short c;
  for ( i = 0; i < textbuff_size; ++i )
  {
    ensure(ucs2buff_size>i*2);
    c = ConvertC2W(text[i],encoding);
    memcpy(ucs2+i,&c,2);
  }
  return (int)i*2;
}

struct OutBitStream{
  unsigned char mask;
  unsigned char* start;
  unsigned char* out;
  unsigned char* out_end;
  OutBitStream(unsigned char* argOut, size_t size){
    mask = 1;
    this->out = argOut;
    this->start = argOut;
    this->out_end = argOut+size;
    memset(argOut,0,size);
  }
  void PutBit(unsigned char bit){
    XMessage(("mask 0x%x, out 0x%x",mask,out));
    if ( mask == 0 ) {
      ensure ( out+1 < out_end );
      ++out;
      mask = 1;
    }
    if ( bit ) *out|=mask;
    mask = mask << 1;
  }
  void Put(unsigned char val){
    XMessage(("Put 0x%x",val));
    val = _8bit_2_7bit[val];
    for ( int i=0; i< 7; ++i ){
      PutBit(val&(1<<i));
    }
  }
  int Size(){
    XMessage(("out %x, size %ld",out,(out-start)+((mask>1)?1:0)));
    return (int)((out-start)+((mask>1)?1:0));
  }
};

struct InBitStream{
  unsigned char mask;
  const unsigned char* start;
  const unsigned char* in;
  const unsigned char* in_end;
  InBitStream(const unsigned char* argIn, size_t size){
    mask = 1;
    this->in = argIn;
    this->start = argIn;
    this->in_end = argIn+size;
  }
  unsigned GetBit(){
    //XMessage(("mask 0x%x, in 0x%x",mask,in));
    if ( mask == 0 ) {
      //ensure ( in+1 < in_end );
      if ( in+1 >= in_end ) return 0xffffffff;
      ++in;
      mask = 1;
    }
    unsigned value = ( *in&mask );
    XMessage(("mask 0x%x, in 0x%x bit : %d",mask,in,value?1:0));
    mask = mask << 1;
    return value;
  }
  unsigned char Get(){
    unsigned char symbol = 0;
    for ( int i=0; i < 7; ++i ){
      unsigned val = GetBit();
      if ( val == 0xffffffff ) return 0;
      if ( val ) symbol|=(1<<i);
    }
    XMessage(("symbol 0x%x, '%c'",symbol,_7bit_2_8bit[symbol&0x7f]));
    return _7bit_2_8bit[symbol&0x7f];
  }
  int WasRead(){
    return (in-start)+(mask>1)?1:0;
  }
};

int RECODE_DECL Convert7BitToText(const char* bit7buf, size_t bit7buf_size,char* text, size_t textbuf_size)
{
  InBitStream bstream((unsigned char*)bit7buf,bit7buf_size);
  size_t i;
  memset(text,0,textbuf_size);
  for ( i=0; i < textbuf_size; ++i ){
    text[i] = bstream.Get();
  }
  return (int)i;
}

int RECODE_DECL ConvertTextTo7Bit(const char* text, size_t textbuf_size, char* bit7buf, size_t bit7buf_size,ConvEncodingEnum encoding)
{
  XMessage(("ConvertTextTo7Bit(_,text:%ld,_,bits:%ld,enc:%d)",textbuf_size,bit7buf_size,encoding));
  OutBitStream bstream((unsigned char*)bit7buf,bit7buf_size);
  bool prev_is_capital;
  if ( textbuf_size > 1 ) {
    if ( (unsigned char)text[1] > 127 ) Translit(text[1],encoding,0,&prev_is_capital);
    else if ( text[1] >= 'A' && text[1] <= 'Z' ) prev_is_capital = true;
    else prev_is_capital = false;
  }
  for ( int i=0; i < textbuf_size; ++i )
  {
    unsigned char ch = (unsigned char)(text[i]);
    if ( ch > 127 ) {
      const unsigned char* sequence = Translit(ch,encoding,prev_is_capital?CONV_PREVIOUS_IS_CAPITAL:0,&prev_is_capital);
      for (; *sequence != 0 ;++sequence){
        bstream.Put(*sequence);
      }
    }else{
      bstream.Put(ch);
      if ( ch >= 'A' && ch <= 'Z' ) prev_is_capital = true;
      else prev_is_capital = false;
    }
  }
  return bstream.Size();
}

int RECODE_DECL Convert7BitToUCS2(const char* bit7buf, size_t bit7buf_size,
                                     short* ucs2, size_t ucs2buff_size)
{
  InBitStream bstream((unsigned char*)bit7buf,bit7buf_size);
  memset(ucs2,0,ucs2buff_size);
  int i;
  for ( i=0; i < ucs2buff_size/2; ++i ){
    ucs2[i] = (short)(unsigned short)bstream.Get();
  }
  return i;
}

int RECODE_DECL ConvertUCS2To7Bit(const short* ucs2, size_t ucs2buff_size,
                                     char* bit7buf, size_t bit7buf_size)
{
  XMessage(("ConvertUCS2To7Bit(_,text:%ld,_,bits:%ld)",ucs2buff_size,bit7buf_size));
  OutBitStream bstream((unsigned char*)bit7buf,bit7buf_size);
  bool prev_is_capital;
  if ( ucs2buff_size >= 4 ) {
    if ( ucs2[1] > 255 ) {
      if ( ucs2[1] > 127 ) Translit(ConvertW2C(ucs2[1],CONV_ENCODING_CP1251),CONV_ENCODING_CP1251,0,&prev_is_capital);
    }else if ( ucs2[1] >= 'A' && ucs2[1] <= 'Z' ) prev_is_capital = true;
    else prev_is_capital = false;
  }
  for ( size_t i=0; i < ucs2buff_size/2; ++i )
  {
    unsigned short ch;
    memcpy(&ch,ucs2+i,2);
    if ( ch > 255 ) {
      const unsigned char* sequence = Translit(ConvertW2C(ch,CONV_ENCODING_CP1251),CONV_ENCODING_CP1251,
                                              prev_is_capital?CONV_PREVIOUS_IS_CAPITAL:0,&prev_is_capital);
      for (; *sequence != 0 ;++sequence){
        bstream.Put(*sequence);
      }
    }else{
      bstream.Put((unsigned char)ch);
      if ( ch >= 'A' && ch <= 'Z' ) prev_is_capital = true;
      else prev_is_capital = false;
    }
  }
  return bstream.Size();
}

unsigned RECODE_DECL ConvertSMSC7BitToLatin1(const char* in, size_t chars,char* out){
  size_t k = 0;
  for(size_t i=0; i<chars; ++i){
    if ( (in[i]&0x7f) == 0x1b ) {
      if ( ++i>=chars ) throw runtime_error("incorrect input buffer");
      switch(in[i]&0x7f){
        case 0x0a: // page break
          out[k++] = '\f'; break;
        case 0x14: //
          out[k++] = '^'; break;
        case 0x1b: // ??? national
          out[k++] = '?'; break;
        case 0x40:
          out[k++] = '|'; break;
        case 0x28: // {
          out[k++] = '{'; break;
        case 0x29: // }
          out[k++] = '}'; break;
        case 0x3c: // [
          out[k++] = '['; break;
        case 0x3d: // ~
          out[k++] = '~'; break;
        case 0x3e: // ]
          out[k++] = ']'; break;
        case 0x2f:
          out[k++] = '\\'; break;
      }
    }else{
      out[k++] = _7bit_2_8bit[in[i]&0x7f];
    }
  }
  return (unsigned)k;
}

unsigned RECODE_DECL ConvertLatin1ToSMSC7Bit(const char* in, size_t chars,char* out){
  size_t k = 0;
  for(size_t i=0; i<chars; ++i){
    switch(in[i]){
    case '^': out[k++] = 0x1b; out[k++] = 0x14; break;
    case '\f':out[k++] = 0x1b; out[k++] = 0x0a; break;
    case '|': out[k++] = 0x1b; out[k++] = 0x40; break;
    case '{': out[k++] = 0x1b; out[k++] = 0x28; break;
    case '}': out[k++] = 0x1b; out[k++] = 0x29; break;
    case '[': out[k++] = 0x1b; out[k++] = 0x3c; break;
    case ']': out[k++] = 0x1b; out[k++] = 0x3e; break;
    case '~': out[k++] = 0x1b; out[k++] = 0x3d; break;
    case '\\':out[k++] = 0x1b; out[k++] = 0x2f; break;
    default:
      out[k++] = _8bit_2_7bit[static_cast<unsigned char>(in[i])];
    }
  }
  return (unsigned)k;
}

#ifdef _WIN32
BOOL APIENTRY DllMain(HANDLE hModule,DWORD  ul_reason_for_call,LPVOID lpReserved)
{
  /*switch (ul_reason_for_call)
  {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
      break;
  }*/
  return 1;
}
#endif
